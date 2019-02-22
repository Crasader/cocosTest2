#include "Geom3D.h"

#include "CC2DMath.h"
#include "CC3DMath.h"
#include <algorithm>
#include "external/poly2tri/poly2tri.h"
#include "cG3DefModelGen.h"

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
    sp->setColor(Color3B(255, 255, 0));
    return addSprite3D(pnode, sp);
}

