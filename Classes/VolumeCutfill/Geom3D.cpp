#include "Geom3D.h"

#include "CC2DMath.h"
#include "CC3DMath.h"
#include "external/poly2tri/poly2tri.h"
#include "cG3DefModelGen.h"

Geom3D::Geom3D()
: model(nullptr)
, modeltop(nullptr)
, modelbottom(nullptr)
, vHeight(0, 0, 0.5f) {

}

Geom3D::~Geom3D() {
    unload();
}

void Geom3D::unload() {
    unloadModel();
}

void Geom3D::poly2tri() {
    int veccnt = (int) vecFree.size();

    std::vector<p2t::Point> points;
    std::vector<p2t::Point *> ppoints;
    points.resize(veccnt);
    ppoints.resize(veccnt);
    for (int i = 0; i < veccnt; i++) {
        points[i].set(vecFree[i].p2d.x, vecFree[i].p2d.y);
        ppoints[i] = &points[i];
    }

    p2t::CDT cdt(ppoints);
    cdt.Triangulate();
    const std::vector<p2t::Triangle *> &tris = cdt.GetTriangles();
    int tricnt = (int) tris.size();
    vecFreeTris.resize(tricnt * 3);
    for (int i = 0; i < tricnt; i++) {
        p2t::Triangle *tri = tris[i];
        vecFreeTris[i * 3].set(tri->GetPoint(0)->x, tri->GetPoint(0)->y);
        vecFreeTris[(i * 3) + 1].set(tri->GetPoint(1)->x, tri->GetPoint(1)->y);
        vecFreeTris[(i * 3) + 2].set(tri->GetPoint(2)->x, tri->GetPoint(2)->y);
    }
}

void Geom3D::unloadModel() {
    CC_SAFE_RELEASE_NULL(model);
    CC_SAFE_RELEASE_NULL(modeltop);
    CC_SAFE_RELEASE_NULL(modelbottom);
}

bool Geom3D::loadModel() {
    unloadModel();
    std::vector<VertexInfo> *clockwisevecs;
    std::vector<VertexInfo> vrev;

    if (isClockwise(vecFree)) {
        vrev = vectormakereverse(vecFree);
        clockwisevecs = &vrev;
    } else {
        clockwisevecs = &vecFree;
    }

    baseCenter = polylist2solid(*clockwisevecs, v3BtmSides, 0);

    modeltop = loadModelTris(vecFreeTris, 0);
    model = loadModelPillarSides(v3BtmSides, vHeight);
    std::vector<Vec2> rev = vectormakereverse(vecFreeTris);
    modelbottom = loadModelTris(rev, 0);

    return true;
}

cG3DefModelGen* Geom3D::loadModelTris(const std::vector<Vec2> &tris, float z) {
    int cntcapvec = (int) tris.size();
    cG3DefModelGen *m = preallocModel(cntcapvec, cntcapvec);

    int cntcaptri = cntcapvec / 3;
    for (int i = 0; i < cntcaptri; i++) {
        int i3 = i * 3;
        modelSetTri(m, i3,
                    poly2solid(tris[i3], z),
                    poly2solid(tris[i3 + 1], z),
                    poly2solid(tris[i3 + 2], z));
    }
    return m;
}

cG3DefModelGen* Geom3D::loadModelPillarSides(const std::vector<VertexInfo> &vecs, const Vec3 sidehigh) {
    int cntside = (int) vecs.size();
    int cntvec = cntside * 4;
    int cntidx = cntside * 6;
    cG3DefModelGen *m = preallocModel(cntvec, cntidx);

    for (int i = 0; i < cntside; i++) {
        modelSetQuatVec(m, i * 6, i * 4, vecs[(i+1)%cntside].p3d + sidehigh, vecs[i].p3d + sidehigh, vecs[(i+1)%cntside].p3d, vecs[i].p3d);
    }
    return m;
}

cG3DefModelGen* loadModelCube(float size) {

    //立方体6个面，每个面4个顶点、6个索引
    cG3DefModelGen* modelGen = preallocModel(6*4, 6*6);

    //立方体的8个顶点
    Vec3 pts[8] = {
            Vec3( -1.0f, -1.0f, -1.0f ),
            Vec3( -1.0f,  1.0f, -1.0f ),
            Vec3(  1.0f,  1.0f, -1.0f ),
            Vec3(  1.0f, -1.0f, -1.0f ),
            Vec3( -1.0f, -1.0f,  1.0f ),
            Vec3( -1.0f,  1.0f,  1.0f ),
            Vec3(  1.0f,  1.0f,  1.0f ),
            Vec3(  1.0f, -1.0f,  1.0f )
    };

    //立方体的6个面及每个面对应的4个顶点
    int f[6][4] = {
            { 0, 1, 2, 3 },
            { 6, 7, 3, 2 },
            { 7, 6, 5, 4 },
            { 0, 4, 5, 1 },
            { 1, 5, 6, 2 },
            { 0, 3, 7, 4 }
    };

    //计算每个顶点的具体坐标
    for (int i = 0; i < 8; i++) {
        pts[i] *= 0.5f * size;
    }

    for (int i = 0; i < 6; i++) { //6个面

        int verStart = i*4; //每个面对应顶点的开始下标
        int idStart = i*6; //每个面索引的开始下标

        for (int j = 0; j < 4; j++) { //每个面的4个顶点
            modelGen->vers[verStart+j].set(pts[f[i][j]]);
        }

        //每个面的4个uv
        modelGen->uvs[verStart] = Vec2(0, 0);
        modelGen->uvs[verStart+1] = Vec2(1, 0);
        modelGen->uvs[verStart+2] = Vec2(1, 1);
        modelGen->uvs[verStart+3] = Vec2(0, 1);

        //每个面的4个法向量
        Vec3 normal = tri2normal(modelGen->vers[verStart+1], modelGen->vers[verStart], modelGen->vers[verStart+2]);
        for (int i = 0; i < 4; i++) {
            modelGen->nors[verStart+i] = normal;
        }

        //每个面的6个索引（每个面分成2个三角形，每个三角形3个索引）
        modelGen->idxs[idStart] = verStart;
        modelGen->idxs[idStart+1] = verStart+1;
        modelGen->idxs[idStart+2] = verStart+2;
        modelGen->idxs[idStart+3] = verStart;
        modelGen->idxs[idStart+4] = verStart+2;
        modelGen->idxs[idStart+5] = verStart+3;
    }

    return modelGen;
}

cG3DefModelGen* loadModelCone(int side, float r, float height) {
    float perAngle = M_PI * 2 / side; //偏移的弧度
    std::vector<Vec3> verts; //底部圆顶点数
    for (int i = 0; i < side; i++) {
        verts.push_back(Vec3(r*cosf(perAngle*(i+1)), r*sinf(perAngle*(i+1)), 0));
    }

    cG3DefModelGen* modelGen = preallocModel(side*3*2, side*3*2);

    //锥环
    Vec3 headVec = Vec3(0, 0, height);
    for (int i = 0; i < side; i++) {
        int verStart = i*3;

        modelGen->vers[verStart] = headVec;
        modelGen->vers[verStart+1] = verts[i];
        modelGen->vers[verStart+2] = verts[(i+1)%side];

        Vec3 normal = tri2normal(verts[i], headVec, verts[(i+1)%verts.size()]);
        modelGen->nors[verStart] = normal;
        modelGen->nors[verStart+1] = normal;
        modelGen->nors[verStart+2] = normal;

        modelGen->uvs[verStart] = Vec2::ZERO;
        modelGen->uvs[verStart+1] = Vec2::UNIT_Y;
        modelGen->uvs[verStart+2] = Vec2::ONE;

        modelGen->idxs[verStart] = verStart;
        modelGen->idxs[verStart+1] = verStart+1;
        modelGen->idxs[verStart+2] = verStart+2;
    }

    //底部圆
    for (int i = side - 1; i > -1; i--) {
        int verStart = side*3+(i*3);

        modelGen->vers[verStart] = Vec3::ZERO;
        modelGen->vers[verStart+1] = verts[i];
        modelGen->vers[verStart+2] = i == 0 ? verts[verts.size()-1] : verts[i-1];

        Vec3 normal = tri2normal(verts[i], Vec3::ZERO, i == 0 ? verts[verts.size()-1] : verts[i-1]);
        modelGen->nors[verStart] = normal;
        modelGen->nors[verStart+1] = normal;
        modelGen->nors[verStart+2] = normal;

        modelGen->uvs[verStart] = Vec2::ZERO;
        modelGen->uvs[verStart+1] = Vec2::UNIT_Y;
        modelGen->uvs[verStart+2] = Vec2::ONE;

        modelGen->idxs[verStart] = verStart;
        modelGen->idxs[verStart+1] = verStart+1;
        modelGen->idxs[verStart+2] = verStart+2;
    }

    return modelGen;
}

cG3DefModelGen* loadModelPyramid(int side, float edge, float height) {
    float perAngle = 2 * M_PI / side;
    float r = 0.5f * edge / sinf(0.5f * perAngle);
    std::vector<Vec3> vects; //顶点集
    for (int i = 0; i < side; i++) {
        vects.push_back(Vec3(r * sinf(perAngle * i), r * cosf(perAngle * i), 0));
    }

    cG3DefModelGen* modelGen = preallocModel(side * 3 + (side - 2) * 3, side * 3 + (side - 2) * 3);

    //底面
    int index = 1;
    Vec3 normal = tri2normal(vects[1], vects[0], vects[2]);
    for (int i = 0; i < side - 2; i++) {
        modelGen->vers[i * 3] = vects[0];
        modelGen->vers[i * 3 + 1] = vects[index];
        modelGen->vers[i * 3 + 2] = vects[(index + 1) % vects.size()];
        index = index + 1;

        modelGen->nors[i * 3] = normal;
        modelGen->nors[i * 3 + 1] = normal;
        modelGen->nors[i * 3 + 2] = normal;

        modelGen->uvs[i * 3] = Vec2::ZERO;
        modelGen->uvs[i * 3 + 1] = Vec2::UNIT_Y;
        modelGen->uvs[i * 3 + 2] = Vec2::ONE;

        modelGen->idxs[i * 3] = i * 3;
        modelGen->idxs[i * 3 + 1] = i * 3 + 1;
        modelGen->idxs[i * 3 + 2] = i * 3 + 2;
    }

    vects = vectormakereverse(vects);

    //侧面-side个侧面
    int verS = (side - 2) * 3;
    Vec3 topVec = Vec3(0, 0, height); //顶部点
    for (int i = 0; i < side; i++) {
        int verStart = verS + i * 3;
        modelGen->vers[verStart] = topVec;
        modelGen->vers[verStart + 1] = vects[i];
        modelGen->vers[verStart + 2] = vects[(i + 1) % vects.size()];

        Vec3 normal = tri2normal(modelGen->vers[verStart + 1], modelGen->vers[verStart], modelGen->vers[verStart + 2]);
        modelGen->nors[verStart] = normal;
        modelGen->nors[verStart + 1] = normal;
        modelGen->nors[verStart + 2] = normal;

        modelGen->uvs[verStart] = Vec2::ZERO;
        modelGen->uvs[verStart + 1] = Vec2::UNIT_Y;
        modelGen->uvs[verStart + 2] = Vec2::ONE;

        modelGen->idxs[verStart] = verStart;
        modelGen->idxs[verStart + 1] = verStart + 1;
        modelGen->idxs[verStart + 2] = verStart + 2;
    }

    return modelGen;
}

cG3DefModelGen* loadModelCylinder(int side, float r, float height) {
    float perAngle = M_PI * 2 / side; //偏移的弧度
    std::vector<Vec3> verts; //底部圆顶点数
    for (int i = 0; i < side; i++) {
        verts.push_back(Vec3(r*cosf(perAngle*(i+1)), r*sinf(perAngle*(i+1)), 0));
    }

    cG3DefModelGen* modelGen = preallocModel(side*3*2+side*4, side*3*2+side*6);

    //顶部圆
    Vec3 topPoint = Vec3(0, 0, height);
    for (int i = 0; i < side; i++) {
        int verStart = i*3;

        modelGen->vers[verStart] = topPoint;
        modelGen->vers[verStart+1] = verts[i] + Vec3(0, 0, height);
        modelGen->vers[verStart+2] = verts[(i+1)%side] + Vec3(0, 0, height);

        Vec3 normal = tri2normal(verts[i] + Vec3(0, 0, height), topPoint, verts[(i+1)%side] + Vec3(0, 0, height));
        modelGen->nors[verStart] = normal;
        modelGen->nors[verStart+1] = normal;
        modelGen->nors[verStart+2] = normal;

        modelGen->uvs[verStart] = Vec2::ZERO;
        modelGen->uvs[verStart+1] = Vec2::UNIT_Y;
        modelGen->uvs[verStart+2] = Vec2::ONE;

        modelGen->idxs[verStart] = verStart;
        modelGen->idxs[verStart+1] = verStart+1;
        modelGen->idxs[verStart+2] = verStart+2;
    }

    //底部圆
    for (int i = side - 1; i > -1; i--) {
        int verStart = side*3+(i*3);

        modelGen->vers[verStart] = Vec3::ZERO;
        modelGen->vers[verStart+1] = verts[i];
        modelGen->vers[verStart+2] = i == 0 ? verts[verts.size()-1] : verts[i-1];

        Vec3 normal = tri2normal(verts[i], Vec3::ZERO, i == 0 ? verts[verts.size()-1] : verts[i-1]);
        modelGen->nors[verStart] = normal;
        modelGen->nors[verStart+1] = normal;
        modelGen->nors[verStart+2] = normal;

        modelGen->uvs[verStart] = Vec2::ZERO;
        modelGen->uvs[verStart+1] = Vec2::UNIT_Y;
        modelGen->uvs[verStart+2] = Vec2::ONE;

        modelGen->idxs[verStart] = verStart;
        modelGen->idxs[verStart+1] = verStart+1;
        modelGen->idxs[verStart+2] = verStart+2;
    }

    //侧边圆环
    for (int i = 0; i < side; i++) {
        int verStart = side*3*2+(i*4); //顶点开始下标
        int idxStart = side*3*2+(i*6); //索引开始下标

        //每个面的4个顶点
        modelGen->vers[verStart] = verts[i];
        modelGen->vers[verStart+1] = verts[(i+1)%side];
        modelGen->vers[verStart+2] = verts[(i+1)%side] + Vec3(0, 0, height);
        modelGen->vers[verStart+3] = verts[i] + Vec3(0, 0, height);

        //每个面的4个法向量
        Vec3 normal = tri2normal(verts[(i+1)%side], verts[i], verts[(i+1)%side] + Vec3(0, 0, height));
        modelGen->nors[verStart] = normal;
        modelGen->nors[verStart+1] = normal;
        modelGen->nors[verStart+2] = normal;
        modelGen->nors[verStart+3] = normal;

        //每个面的4个uv
        modelGen->uvs[verStart] = Vec2::ZERO;
        modelGen->uvs[verStart+1] = Vec2::UNIT_X;
        modelGen->uvs[verStart+2] = Vec2::ONE;
        modelGen->uvs[verStart+3] = Vec2::UNIT_Y;

        //每个面的6个索引（每个面分成2个三角形，每个三角形3个索引）
        modelGen->idxs[idxStart] = verStart;
        modelGen->idxs[idxStart+1] = verStart+1;
        modelGen->idxs[idxStart+2] = verStart+2;
        modelGen->idxs[idxStart+3] = verStart;
        modelGen->idxs[idxStart+4] = verStart+2;
        modelGen->idxs[idxStart+5] = verStart+3;
    }

    return modelGen;
}

cG3DefModelGen* loadModelSphere(float r, int latCount, int lonCount) {
    float beta = M_PI_2;
    const float alpha_inc = 2.0f * M_PI / (lonCount-1);
    const float beta_inc = -M_PI / (latCount-1);

    int maxIndex = latCount * lonCount; //总顶点数

    std::vector<Vec3> vects; //顶点集
    std::map<int, std::vector<int>> faces; //面map

    int vIndex = 0;
    for(int j = 0; j < latCount; j++) {
        float alpha = 0.0f;
        for (int i = 0; i < lonCount; i++) {
            float x = r * cos(beta) * cos(alpha);
            float y = r * sin(beta);
            float z = r * cos(beta) * sin(alpha);
            Vec3 p = Vec3(x, y, z);
            vects.push_back(p);

            std::vector<int> vs;
            int id0 = vIndex % maxIndex;
            int id1 = (id0 + 1) % maxIndex;
            int id2 = (id1 + lonCount) % maxIndex;
            int id3 = (id0 + lonCount) % maxIndex;
            vs.push_back(id0);
            vs.push_back(id1);
            vs.push_back(id2);
            vs.push_back(id3);
            faces.insert(std::make_pair(vIndex, vs));

            alpha += alpha_inc;
            vIndex++;
        }
        beta += beta_inc;
    }

    //每个面4个顶点、6个索引
    int faceCount = faces.size();
    cG3DefModelGen* modelGen = preallocModel(faceCount*4, faceCount*6);

    for (auto it = faces.begin(); it != faces.end(); it++) {
        int index = it->first;
        std::vector<int> vs = it->second;

        int verStart = index*4;
        int idStart = index*6;

        //每个面的4个顶点
        for (int i = 0; i < 4; i++) {
            modelGen->vers[verStart+i] = vects[vs[i]];
        }

        //每个面的4个uvs
        modelGen->uvs[verStart] = Vec2::ZERO;
        modelGen->uvs[verStart+1] = Vec2::UNIT_X;
        modelGen->uvs[verStart+2] = Vec2::ONE;
        modelGen->uvs[verStart+3] = Vec2::UNIT_Y;

        //每个面的4个法向量
        Vec3 normal = tri2normal(vects[vs[1]], vects[vs[0]], vects[vs[2]]);
        for (int i = 0; i < 4; i++) {
            modelGen->nors[verStart+i] = normal;
        }

        //每个面的6个索引（每个面分成2个三角形，每个三角形3个索引）
        modelGen->idxs[idStart] = verStart;
        modelGen->idxs[idStart+1] = verStart+1;
        modelGen->idxs[idStart+2] = verStart+2;
        modelGen->idxs[idStart+3] = verStart;
        modelGen->idxs[idStart+4] = verStart+2;
        modelGen->idxs[idStart+5] = verStart+3;
    }

    return modelGen;
}

cG3DefModelGen* preallocModel(int cntvec, int cntidx) {
    cG3DefModelGen* m = new cG3DefModelGen("Solid");
    m->num_verts = cntvec;
    m->nors.resize(cntvec);
    m->vers.resize(cntvec);
    m->uvs.resize(cntvec);

    m->num_idx = cntidx;
    m->idxs.resize(cntidx);
    return m;
}

Sprite3D* addSprite3DModel(cG3DefModelGen* mdl, Node* pnode, const Vec3 &pos) {
    Sprite3D* sp = DefModel2Sprite3D(mdl);
    sp->setTexture("white2.png");
    sp->setPosition3D(pos);
    sp->setColor(Color3B(255, 184, 156));
    return addSprite3D(pnode, sp);
}

