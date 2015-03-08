/* Copyright (C) 2006 - 2011 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Duskwood
SD%Complete:
SDComment: Quest support: A daughter's love, outro. Translation to Ello, stitches event.
SDCategory: Duskwood
EndScriptData */

/* ContentData
EndContentData */

#include "precompiled.h"

/*######
## npc_town_crier
######*/

struct MANGOS_DLL_DECL npc_town_crierAI : public ScriptedAI							// used for zone yells on stitches event and dodds and paige
{
    npc_town_crierAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
		m_creature->SetActiveObjectState(true);
        Reset();
    }

    void Reset()
    {
    }

	void UpdateAI(const uint32 uiDiff)
    {
		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_town_crier(Creature* pCreature)
{
    return new npc_town_crierAI(pCreature);
}

/*######
## npc_morgan_ladimore
######*/

enum
{
	SAY_1 = -1720010,
	SAY_2 = -1720011,
};

struct MANGOS_DLL_DECL npc_morgan_ladimoreAI : public ScriptedAI
{
    npc_morgan_ladimoreAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

	uint32 m_uiSpeechTimer;
    uint8 m_uiSpeechStep;

    void Reset()
    {
		m_creature->SetStandState(UNIT_STAND_STATE_STAND);
		m_uiSpeechTimer = 0;
        m_uiSpeechStep = 1;
    }


    void UpdateAI(const uint32 uiDiff)					// handle Rp at end of A daughter's love quest, it's a hack but can't do it in DB
    {
		if (!m_uiSpeechStep)
            return;

        if (m_uiSpeechTimer < uiDiff)
        {
            switch(m_uiSpeechStep)
            {
                case 1:
                    m_creature->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
                    m_uiSpeechTimer = 3000;
                    break;
                case 2:
                    DoScriptText(SAY_1, m_creature);
                    m_uiSpeechTimer = 5000;
                    break;
				case 3:
					DoScriptText(SAY_2, m_creature);
                    m_uiSpeechTimer = 3000;
                    break;
				case 4:
					m_creature->SetStandState(UNIT_STAND_STATE_KNEEL);
					m_uiSpeechStep = 0;
					return;

                default:
                    m_uiSpeechStep = 0;
                    return;
            }
            ++m_uiSpeechStep;
        }
        else
            m_uiSpeechTimer -= uiDiff;
    }

};

CreatureAI* GetAI_npc_morgan_ladimore(Creature* pCreature)
{
    return new npc_morgan_ladimoreAI(pCreature);
}

/*######
## mob_stitches
######*/

enum
{
	YELL_SPAWN							= -1720020,
	YELL_AGGRO							= -1720021,
	YELL_GUARD_DOWN						= -1720022,
	YELL_STITCHES_DOWN					= -1720023,

	SPELL_AURA_OF_ROT					= 3106,

	NPC_WATCHER_DODDS					= 888,			// Crossroads group
	NPC_WATCHER_PAIGE					= 499,
	NPC_WATCHER_BLOMBERG				= 1000,
	NPC_WATCHER_HUTCHINS				= 1001,

	NPC_WATCHER_CUTFORD					= 1436,

	NPC_WATCHER_SELKIN					= 1100,			// rotting orchard group
	NPC_WATCHER_GELWIN					= 1099,
	NPC_WATCHER_MERANT					= 1098,
	NPC_WATCHER_THAYER					= 1101,
	
	NPC_WATCHER_CORWIN					= 1204,
	NPC_WATCHER_SARYS					= 1203,

	NPC_TOWN_CRIER						= 468,
	NPC_STITCHES						= 412,
};

struct Loc
{
    float x, y, z, o;
};

static Loc aXRoadSpawnLoc[]= 
{
	{-10907.0f, -387.0f, 40.76f, 1.365f},
	{-10901.0f, -388.0f, 40.76f, 1.345f},
	{-10903.0f, -391.0f, 40.93f, 1.345f},			// move loc for dodds
	{-10905.0f, -391.45f, 40.93f, 1.345f},			// move loc for paige
};

static Loc aDarkshireSpawnLoc[]= 
{
	{-10696.0f, -1178.0f, 26.76f, 2.57f},		// right
	{-10698.0f, -1181.0f, 26.96f, 2.57f},
	{-10699.0f, -1183.0f, 26.93f, 2.57f},
	{-10701.0f, -1185.0f, 26.96f, 2.57f},		// left
};
static uint32 aFirstWatcherID[]=
{
	{NPC_WATCHER_BLOMBERG},
	{NPC_WATCHER_HUTCHINS},
};

static uint32 aWatcherID[]=
{
	{NPC_WATCHER_SELKIN},
	{NPC_WATCHER_GELWIN},
	{NPC_WATCHER_MERANT},
	{NPC_WATCHER_THAYER},
};

struct MANGOS_DLL_DECL mob_stitchesAI : public ScriptedAI
{
    mob_stitchesAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
		m_creature->SetActiveObjectState(true); 
		//m_creature->MonsterYellToZone(YELL_SPAWN, LANG_UNIVERSAL, NULL);			// no need to force zone yell, it's in DB for all the ones that should
		DoScriptText(YELL_SPAWN, m_creature);			// yell on spawn
		m_bFirstGuardDown = false;
		m_bStitchesDown = false;					// don't reset these 

		Creature* pStitch = GetClosestCreatureWithEntry(m_creature, NPC_STITCHES, 1000.0f);			// not working, can spawn an army of stitches!
			if (pStitch)
				pStitch->ForcedDespawn();

        Reset();
    }

	uint32 m_uiAuraofRotTimer;
	uint32 m_uiYellTimer;

	bool m_bFirstGuardDown;
	bool m_bStitchesDown;

    void Reset()
    {
		m_uiYellTimer = 0;
		m_uiAuraofRotTimer = urand(5000, 12000);
    }

	void Aggro(Unit* /*pWho*/)
    {
        DoScriptText(YELL_AGGRO, m_creature);
    }

	void JustSummoned(Creature* pSummoned)
    {
		pSummoned->SetRespawnDelay(-10);			// make sure they won't respawn randomly
	}

	void JustDied(Unit* /*pKiller*/)
    {
		HandleYell(YELL_STITCHES_DOWN);
	}
	void SummonedCreatureJustDied(Creature* pSummoned)
    {
		if (pSummoned->GetEntry() == aFirstWatcherID[0,1])
			if (!m_bFirstGuardDown)
				HandleYell(YELL_GUARD_DOWN);
	}

	void MovementInform(uint32 /*uiMotiontype*/, uint32 uiPointId)
	{
		switch(uiPointId)
        {
			case 12:			// spawn cutford who runs all the way from darkshire to meet the guards at crossroads
				m_creature->SummonCreature(NPC_WATCHER_CUTFORD, aDarkshireSpawnLoc[1].x, aDarkshireSpawnLoc[1].y, aDarkshireSpawnLoc[1].z, aDarkshireSpawnLoc[1].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 180000, true);
				break;
			case 14:			// move guards from the camp up to the road
				if (Creature* pDodds = GetClosestCreatureWithEntry(m_creature, NPC_WATCHER_DODDS, 100.0f))		// only working SOME TIMES and if player is near, even tho they have activeobject
					pDodds->GetMotionMaster()->MovePoint(1,-10903.0f, -391.0f, 40.93f);

				if (Creature* pPaige = GetClosestCreatureWithEntry(m_creature, NPC_WATCHER_PAIGE, 100.0f))		// only working SOME TIMES and if player is near, even tho they have activeobject
					pPaige->GetMotionMaster()->MovePoint(1,-10905.0f, -391.45f, 40.93f);

				break;
			case 15:			// turn them the right way, they wouldn't move if it were in case 14
				if (Creature* pDodds = GetClosestCreatureWithEntry(m_creature, NPC_WATCHER_DODDS, 100.0f))
					pDodds->SetFacingTo(1.345f);

				if (Creature* pPaige = GetClosestCreatureWithEntry(m_creature, NPC_WATCHER_PAIGE, 100.0f))
					pPaige->SetFacingTo(1.345f);

				break;
			case 16:			// spawn the guards at crossroads
				for(uint8 i = 0; i < 2; ++i)
					m_creature->SummonCreature(aFirstWatcherID[i], aXRoadSpawnLoc[i].x, aXRoadSpawnLoc[i].y, aXRoadSpawnLoc[i].z, aXRoadSpawnLoc[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 180000, true);
				
				break;
			case 25:			// spawn the guards who run out from darkshire to rotting orchard
				for(uint8 i = 0; i < 4; ++i)
					m_creature->SummonCreature(aWatcherID[i], aDarkshireSpawnLoc[i].x, aDarkshireSpawnLoc[i].y, aDarkshireSpawnLoc[i].z, aDarkshireSpawnLoc[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 300000, true);
				break;
			case 55:			// spawn the last set of guards, just outside darkshire
				m_creature->SummonCreature(NPC_WATCHER_CORWIN, -10314.09f, -1137.0f, 21.99f, 2.41f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 180000, true);
				m_creature->SummonCreature(NPC_WATCHER_SARYS, -10310.0f, -1133.0f, 22.25f, 2.41f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 180000, true);
				break;
		}
	}

	void HandleYell(uint32 uiYellId = 0)			// handle the yells, THIS CAN CRASH THE SERVER!
	{
		Creature* pCrier = GetClosestCreatureWithEntry(m_creature, NPC_TOWN_CRIER, 1000.0f); 			// make sure we get him! that's one hell of a distance		
		{	
			if (uiYellId == YELL_GUARD_DOWN && !m_bFirstGuardDown)
			{
				/*if (npc_town_crierAI* pCrierAI = dynamic_cast<npc_town_crierAI*>(pCrier->AI()))*/	 // this crashes server lol
				//pCrier->MonsterYellToZone(YELL_GUARD_DOWN, LANG_UNIVERSAL, NULL);
				if (pCrier)
					DoScriptText(YELL_GUARD_DOWN, pCrier);												// Not working, crier won't yell unless within a short distance(around rotting orchard, max)
				m_bFirstGuardDown = true;
			}

			if (uiYellId == YELL_STITCHES_DOWN && !m_bStitchesDown)
			{
				/*if (npc_town_crierAI* pCrierAI = dynamic_cast<npc_town_crierAI*>(pCrier->AI()))*/
				//pCrier->MonsterYellToZone(YELL_STITCHES_DOWN, LANG_UNIVERSAL, NULL);
				if (pCrier)
					DoScriptText(YELL_STITCHES_DOWN, pCrier);											// Not working, crier won't yell unless within a short distance(around rotting orchard, max)
				m_bStitchesDown = true;
			}
		}
	}

	void UpdateAI(const uint32 uiDiff)
    {
		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

		// Aura of Rot
        if (m_uiAuraofRotTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_AURA_OF_ROT);
            m_uiAuraofRotTimer = urand(5000,12000);
        }
        else
            m_uiAuraofRotTimer -= uiDiff;

        DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_mob_stitches(Creature* pCreature)
{
    return new mob_stitchesAI(pCreature);
}


void AddSC_duskwood()
{
    Script* pNewscript;

	pNewscript = new Script;
	pNewscript->Name = "npc_town_crier";
	pNewscript->GetAI = &GetAI_npc_town_crier;
	pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_morgan_ladimore";
    pNewscript->GetAI = &GetAI_npc_morgan_ladimore;
    pNewscript->RegisterSelf();

	pNewscript = new Script;
	pNewscript->Name = "mob_stitches";
	pNewscript->GetAI = &GetAI_mob_stitches;
	pNewscript->RegisterSelf();
}
