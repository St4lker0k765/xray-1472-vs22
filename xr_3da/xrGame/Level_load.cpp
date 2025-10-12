#include "stdafx.h"
#include "HUDmanager.h"
#include "LevelGameDef.h"
#include "ai_space.h"
#include "ai\ai_selector_template.h"
#include "..\PGObject.h"

void CLevel::vfCreateAllPossiblePaths(string64 sName, SPath &tpPatrolPath)
{
    std::vector<BYTE>    tpaFrom;
    std::vector<BYTE>    tpaTo;
    std::vector<Fvector> tpaPoints;
    std::vector<Fvector> tpaDeviations;
    std::vector<u32>     tpaNodes;

    int  iStartPoint = -1, iFinishPoint = -1;
    int  iCurPoint = 0, iPrevPoint = -1;

    const u32 N = static_cast<u32>(tpPatrolPath.tpaWayPoints.size());
    u32 dwOneZero = 0, dwZeroOne = 0, dwOneCount = 0, dwTwoCount = 0;

    tpaFrom.assign(N, 0);
    tpaTo.assign(N, 0);

    tpPatrolPath.dwType = PATH_LOOPED | PATH_BIDIRECTIONAL;

    // computing from-to arrays
    for (u32 i = 0; i < static_cast<u32>(tpPatrolPath.tpaWayLinks.size()); ++i) {
        const auto &L = tpPatrolPath.tpaWayLinks[i];
        ++tpaTo[L.wTo];
        ++tpaFrom[L.wFrom];
    }

    // counting types of points
    for (u32 ui = 0; ui < N; ++ui) {
        const int i = static_cast<int>(ui);
        if (tpaTo[ui] > 2)
            Debug.fatal("Patrol path %s : invalid count of incoming links (%d) for point %d [%.2f,%.2f,%.2f]",
                        sName, tpaTo[ui], i,
                        tpPatrolPath.tpaWayPoints[i].tWayPoint.x,
                        tpPatrolPath.tpaWayPoints[i].tWayPoint.y,
                        tpPatrolPath.tpaWayPoints[i].tWayPoint.z);
        if (tpaFrom[ui] > 2)
            Debug.fatal("Patrol path %s : invalid count of outcoming links (%d) for point %d [%.2f,%.2f,%.2f]",
                        sName, tpaFrom[ui], i,
                        tpPatrolPath.tpaWayPoints[i].tWayPoint.x,
                        tpPatrolPath.tpaWayPoints[i].tWayPoint.y,
                        tpPatrolPath.tpaWayPoints[i].tWayPoint.z);

        if ((tpaTo[ui] == 1) && (tpaFrom[ui] == 0)) {
            if (dwOneZero)
                Debug.fatal("Patrol path %s : invalid count of start points [%.2f,%.2f,%.2f]",
                            sName,
                            tpPatrolPath.tpaWayPoints[i].tWayPoint.x,
                            tpPatrolPath.tpaWayPoints[i].tWayPoint.y,
                            tpPatrolPath.tpaWayPoints[i].tWayPoint.z);
            ++dwOneZero;
            iFinishPoint = i;
        }

        if ((tpaTo[ui] == 0) && (tpaFrom[ui] == 1)) {
            if (dwZeroOne)
                Debug.fatal("Patrol path %s : invalid count of finish points [%.2f,%.2f,%.2f]",
                            sName,
                            tpPatrolPath.tpaWayPoints[i].tWayPoint.x,
                            tpPatrolPath.tpaWayPoints[i].tWayPoint.y,
                            tpPatrolPath.tpaWayPoints[i].tWayPoint.z);
            ++dwZeroOne;
            iStartPoint = i;
        }

        if ((tpaTo[ui] == 1) && (tpaFrom[ui] == 1)) {
            if ((!dwOneCount) && (!dwZeroOne) && (!dwOneZero))
                iStartPoint = i;
            else
                if ((dwOneCount == 1) && (!dwZeroOne) && (!dwOneZero))
                    iFinishPoint = i;
            ++dwOneCount;
        }

        if ((tpaTo[ui] == 2) && (tpaFrom[ui] == 2))
            ++dwTwoCount;
    }

    // checking for supported path types
    if (!(dwOneZero + dwZeroOne)) {
        if ((dwOneCount == 2) && (dwTwoCount == N - 2)) {
            iCurPoint = iStartPoint;
            tpPatrolPath.dwType ^= PATH_LOOPED;
        } else if (dwOneCount == N) {
            tpPatrolPath.dwType ^= PATH_BIDIRECTIONAL;
        } else if (dwTwoCount != N) {
            const int di = (N ? 0 : -1);
            Debug.fatal("Patrol path %s : invalid count of outcoming links (%d) for point %d [%.2f,%.2f,%.2f]",
                        sName,
                        di >= 0 ? tpaFrom[di] : -1,
                        di,
                        di >= 0 ? tpPatrolPath.tpaWayPoints[di].tWayPoint.x : 0.f,
                        di >= 0 ? tpPatrolPath.tpaWayPoints[di].tWayPoint.y : 0.f,
                        di >= 0 ? tpPatrolPath.tpaWayPoints[di].tWayPoint.z : 0.f);
        }
    } else {
        if ((dwOneCount != N - 2) || (dwOneZero != 1) || (dwZeroOne != 1)) {
            const int di = (N ? 0 : -1);
            Debug.fatal("Patrol path %s : invalid count of outcoming links (%d) for point %d [%.2f,%.2f,%.2f] in non-looped one-directional path",
                        sName,
                        di >= 0 ? tpaFrom[di] : -1,
                        di,
                        di >= 0 ? tpPatrolPath.tpaWayPoints[di].tWayPoint.x : 0.f,
                        di >= 0 ? tpPatrolPath.tpaWayPoints[di].tWayPoint.y : 0.f,
                        di >= 0 ? tpPatrolPath.tpaWayPoints[di].tWayPoint.z : 0.f);
        } else {
            iCurPoint = iStartPoint;
            tpPatrolPath.dwType ^= (PATH_LOOPED ^ PATH_BIDIRECTIONAL);
        }
    }

    // building point sequences and path
    tpPatrolPath.tpaWayPointIndexes.resize(N);
    for (u32 ui = 0; ui < N; ++ui) {
        tpPatrolPath.tpaWayPointIndexes[ui] = static_cast<u16>(iCurPoint);
        for (u32 lj = 0; lj < static_cast<u32>(tpPatrolPath.tpaWayLinks.size()); ++lj) {
            const auto &L = tpPatrolPath.tpaWayLinks[lj];
            if ((L.wFrom == iCurPoint) && (L.wTo != iPrevPoint)) {
                iPrevPoint = iCurPoint;
                iCurPoint = L.wTo;
                break;
            }
        }
    }

    // creating realistic path
    tpaDeviations.resize(N);
    tpaPoints.resize(N);
    for (u32 ui = 0; ui < N; ++ui)
        tpaPoints[ui] = tpPatrolPath.tpaWayPoints[tpPatrolPath.tpaWayPointIndexes[ui]].tWayPoint;

    getAI().vfCreateFastRealisticPath(
        tpaPoints,
        tpPatrolPath.tpaWayPoints[tpPatrolPath.tpaWayPointIndexes[0]].dwNodeID,
        tpaDeviations,
        tpPatrolPath.tpaVectors[0],
        tpaNodes,
        (tpPatrolPath.dwType & PATH_LOOPED) != 0
    );

    // creating variations
    if (tpPatrolPath.tpaVectors[0].empty())
        Debug.fatal("Patrol path %s was not built - there are not enough nodes to build all the straight lines", sName);

    tpPatrolPath.tpaVectors[1].resize(tpPatrolPath.tpaVectors[0].size());
    tpPatrolPath.tpaVectors[2].resize(tpPatrolPath.tpaVectors[0].size());

    const float fHalfSubnodeSize = getAI().Header().size * 0.5f;

    std::vector<Fvector> &tpaVector0 = tpPatrolPath.tpaVectors[0];
    const u32 M = static_cast<u32>(tpaVector0.size());

    for (int I = 1; I < 3; ++I) {
        std::vector<Fvector> &tpaVector1 = (I == 1) ? tpPatrolPath.tpaVectors[1] : tpPatrolPath.tpaVectors[2];

        int j = 0;
        int k = 0;

        for (u32 ui = 0; ui < M; ++ui) {
            const int i = static_cast<int>(ui);

            if (static_cast<size_t>(j) >= tpaVector1.size())
                tpaVector1.resize(static_cast<size_t>(j) + 1);

            tpaVector1[j] = tpaVector0[i];

            Fvector tTemp;

            if (tpPatrolPath.dwType & PATH_LOOPED) {
                const int next1 = (i < static_cast<int>(M) - 1) ? (i + 1) : 0;
                tTemp.sub(tpaVector0[next1], tpaVector0[i]);
                tTemp.y = 0.f;
                if (tTemp.magnitude() < EPS_L) {
                    const int next2 = (i < static_cast<int>(M) - 2) ? (i + 2) : 1;
                    tTemp.sub(tpaVector0[next2], tpaVector0[i]);
                    tTemp.y = 0.f;
                }
            } else {
                if (i < static_cast<int>(M) - 1) {
                    const int next1 = (i < static_cast<int>(M) - 1) ? (i + 1) : 0;
                    tTemp.sub(tpaVector0[next1], tpaVector0[i]);
                } else {
                    tTemp.sub(tpaVector0[i], tpaVector0[i - 1]);
                }
                tTemp.y = 0.f;
            }
            tTemp.normalize();

            if (I == 1)
                tTemp.set(tTemp.z, 0.f, -tTemp.x);
            else
                tTemp.set(-tTemp.z, 0.f, tTemp.x);

            tpaVector1[j].add(tTemp);

            const int k_start = k;
            while ((k < static_cast<int>(tpaNodes.size())) &&
                   (!getAI().bfInsideNode(getAI().Node(tpaNodes[k]), tpaVector0[i])))
            {
                ++k;
            }

            if (k >= static_cast<int>(tpaNodes.size())) {
                k = k_start;
                tpaVector1.erase(tpaVector1.begin() + j);
                continue;
            }

            CAI_NodeEvaluatorTemplate<aiSearchRange | aiInsideNode> tSearch;
            tSearch.m_fSearchRange   = 4 * fHalfSubnodeSize;
            tSearch.m_dwStartNode    = tpaNodes[k];
            tSearch.m_tStartPosition = tpaVector0[i];
            tSearch.vfShallowGraphSearch(getAI().q_mark_bit_x);
            tpaVector1[j].y = getAI().ffGetY(*(getAI().Node(tSearch.m_dwBestNode)),
                                             tpaVector1[j].x, tpaVector1[j].z);
            ++j;
        }

        if (!tpaVector1.empty() &&
            (tpaVector1[0].distance_to(tpaVector1[static_cast<size_t>(j) - 1]) > EPS_L))
        {
            tpaVector1.push_back(tpaVector1[0]);
            ++j;
        }

        if (j >= 0)
            tpaVector1.resize(static_cast<size_t>(j));
    }

    if ((tpPatrolPath.dwType & PATH_LOOPED) &&
        (tpaVector0[0].distance_to(tpaVector0[tpaVector0.size() - 1]) > EPS_L))
    {
        tpaVector0.push_back(tpaVector0[0]);
    }
}


BOOL CLevel::Load_GameSpecific_Before()
{
	// AI space
	pApp->LoadTitle	("Loading AI objects...");
	getAI().Load	();

	string256		fn_game;
	if (FS.exist(fn_game, "$level$", "level.game")) {
		IReader *F = FS.r_open	(fn_game);
		IReader *O = 0;

		// Load WayPoints
		if (0!=(O = F->open_chunk	(WAY_PATROLPATH_CHUNK))) { 
			int chunk = 0;
			for (IReader *OBJ = O->open_chunk(chunk++); OBJ; OBJ = O->open_chunk(chunk++)) {
				R_ASSERT(OBJ->find_chunk(WAYOBJECT_CHUNK_VERSION));
				u32 dw = OBJ->r_u16();
				R_ASSERT(dw == WAYOBJECT_VERSION);

				SPath tPatrolPath;

				string64 sName;
				R_ASSERT(OBJ->find_chunk(WAYOBJECT_CHUNK_NAME));
				OBJ->r_stringZ(sName);

				R_ASSERT(OBJ->find_chunk(WAYOBJECT_CHUNK_POINTS));
				u32 dwCount = OBJ->r_u16();
				tPatrolPath.tpaWayPoints.resize(dwCount);
				for (int i=0; i<(int)dwCount; i++){
					OBJ->r_fvector3(tPatrolPath.tpaWayPoints[i].tWayPoint);
					tPatrolPath.tpaWayPoints[i].dwFlags = OBJ->r_u32();
					tPatrolPath.tpaWayPoints[i].dwNodeID = getAI().q_LoadSearch(tPatrolPath.tpaWayPoints[i].tWayPoint);
				}

				R_ASSERT(OBJ->find_chunk(WAYOBJECT_CHUNK_LINKS));
				u32 dwCountL = OBJ->r_u16();
				tPatrolPath.tpaWayLinks.resize(dwCountL);
				for (int i=0; i<(int)dwCountL; i++){
					tPatrolPath.tpaWayLinks[i].wFrom = OBJ->r_u16();
					tPatrolPath.tpaWayLinks[i].wTo = OBJ->r_u16();
				}

				OBJ->close();

				// sorting links
				bool bOk;
				do {
					bOk = true;
					for (int i=1; i<(int)dwCountL; i++)
						if ((tPatrolPath.tpaWayLinks[i - 1].wFrom > tPatrolPath.tpaWayLinks[i].wFrom) || ((tPatrolPath.tpaWayLinks[i - 1].wFrom == tPatrolPath.tpaWayLinks[i].wFrom) && (tPatrolPath.tpaWayLinks[i - 1].wTo > tPatrolPath.tpaWayLinks[i].wTo))) {
							WORD wTemp = tPatrolPath.tpaWayLinks[i - 1].wFrom;
							tPatrolPath.tpaWayLinks[i - 1].wFrom = tPatrolPath.tpaWayLinks[i].wFrom;
							tPatrolPath.tpaWayLinks[i].wFrom = wTemp;
							wTemp = tPatrolPath.tpaWayLinks[i - 1].wTo;
							tPatrolPath.tpaWayLinks[i - 1].wTo = tPatrolPath.tpaWayLinks[i].wTo;
							tPatrolPath.tpaWayLinks[i].wTo = wTemp;
							bOk = false;
						}
				}
				while (!bOk);

				m_PatrolPaths[sName] = tPatrolPath;
				
				vfCreateAllPossiblePaths(sName, m_PatrolPaths[sName]);
			}
			O->close();
		}
		FS.r_close(F);
	}

	return TRUE;
}

BOOL CLevel::Load_GameSpecific_After()
{
	// loading static particles
	string256		fn_game;
	if (FS.exist(fn_game, "$level$", "level.ps_static")) {
		IReader *F = FS.r_open	(fn_game);
		IRender_Sector* S;
		CPGObject*		pStaticPG;
		int				chunk = 0;
		string256		ref_name;
		Fmatrix			transform;
		for (IReader *OBJ = F->open_chunk(chunk++); OBJ; OBJ = F->open_chunk(chunk++)){
			OBJ->r_stringZ				(ref_name);
			OBJ->r						(&transform,sizeof(Fmatrix));transform.c.y+=0.01f;
			S							= ::Render->detectSector	(transform.c);
			pStaticPG					= xr_new<CPGObject>			(ref_name,S,false);
			pStaticPG->UpdateParent		(transform);
			pStaticPG->Play				();
			m_StaticParticles.push_back	(pStaticPG);
		}
	}
	return TRUE;
}

void CLevel::Load_GameSpecific_CFORM	( CDB::TRI* tris, u32 count )
{
	// 1.
	u16		default_id	= (u16)GMLib.GetMaterialIdx("default");

	// 2. Build mapping
	map<u32,u16>		translator;
	translator.insert	(make_pair(u32(-1),default_id));
	u16 idx				= 0;
	for (GameMtlIt I=GMLib.FirstMaterial(); I!=GMLib.LastMaterial(); I++)
	{
		translator.insert(make_pair((*I)->GetID(),idx++));
	}

	// 3.
	for (u32 it=0; it<count; it++)
	{
		CDB::TRI* T						= tris + it;
		map<u32,u16>::iterator index	= translator.find(T->dummy);
		if (index==translator.end())	Debug.fatal	("Game material '%d' not found",T->dummy);
		T->material						= index->second;
	}
}
