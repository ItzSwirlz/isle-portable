#include "ambulance.h"

#include "decomp.h"
#include "isle.h"
#include "isle_actions.h"
#include "jukebox_actions.h"
#include "legoanimationmanager.h"
#include "legocontrolmanager.h"
#include "legogamestate.h"
#include "legonavcontroller.h"
#include "legopathstruct.h"
#include "legoutils.h"
#include "legovariables.h"
#include "legoworld.h"
#include "misc.h"
#include "mxactionnotificationparam.h"
#include "mxbackgroundaudiomanager.h"
#include "mxmisc.h"
#include "mxsoundpresenter.h"
#include "mxticklemanager.h"
#include "mxtimer.h"
#include "mxtransitionmanager.h"
#include "mxvariabletable.h"
#include "scripts.h"

#include <SDL3/SDL_stdinc.h>
#include <stdio.h>

DECOMP_SIZE_ASSERT(Ambulance, 0x184)
DECOMP_SIZE_ASSERT(AmbulanceMissionState, 0x24)

// Flags used in isle.cpp
extern MxU32 g_isleFlags;

// FUNCTION: LEGO1 0x10035ee0
// FUNCTION: BETA10 0x10022820
Ambulance::Ambulance()
{
	m_maxLinearVel = 40.0;
	m_state = NULL;
	m_unk0x168 = 0;
	m_actorId = -1;
	m_atPoliceTask = 0;
	m_atBeachTask = 0;
	m_taskState = Ambulance::e_none;
	m_lastAction = IsleScript::c_noneIsle;
	m_enableRandomAudio = 0;
	m_lastAnimation = IsleScript::c_noneIsle;
	m_fuel = 1.0;
}

// FUNCTION: LEGO1 0x10036150
// FUNCTION: BETA10 0x100228fe
Ambulance::~Ambulance()
{
	ControlManager()->Unregister(this);
	TickleManager()->UnregisterClient(this);
}

// FUNCTION: LEGO1 0x100361d0
// FUNCTION: BETA10 0x10022993
MxResult Ambulance::Create(MxDSAction& p_dsAction)
{
	MxResult result = IslePathActor::Create(p_dsAction);

	if (result == SUCCESS) {
		m_world = CurrentWorld();

		if (m_world) {
			m_world->Add(this);
		}

		m_state = (AmbulanceMissionState*) GameState()->GetState("AmbulanceMissionState");
		if (!m_state) {
			m_state = new AmbulanceMissionState();
			m_state->m_state = AmbulanceMissionState::e_ready;
			GameState()->RegisterState(m_state);
		}
	}

	VariableTable()->SetVariable(g_varAMBULFUEL, "1.0");
	m_fuel = 1.0;
	m_time = Timer()->GetTime();
	return result;
}

// FUNCTION: LEGO1 0x10036300
void Ambulance::Animate(float p_time)
{
	IslePathActor::Animate(p_time);

	if (UserActor() == this) {
		char buf[200];
		float speed = abs(m_worldSpeed);
		float maxLinearVel = NavController()->GetMaxLinearVel();

		sprintf(buf, "%g", speed / maxLinearVel);
		VariableTable()->SetVariable(g_varAMBULSPEED, buf);

		m_fuel += (p_time - m_time) * -3.333333333e-06f;
		if (m_fuel < 0) {
			m_fuel = 0;
		}

		m_time = p_time;

		sprintf(buf, "%g", m_fuel);
		VariableTable()->SetVariable(g_varAMBULFUEL, buf);
	}
}

// FUNCTION: LEGO1 0x100363f0
// FUNCTION: BETA10 0x10022b2a
void Ambulance::CreateState()
{
	LegoGameState* gameState = GameState();
	AmbulanceMissionState* state = (AmbulanceMissionState*) gameState->GetState("AmbulanceMissionState");

	if (state == NULL) {
		state = (AmbulanceMissionState*) gameState->CreateState("AmbulanceMissionState");
	}

	m_state = state;
}

// FUNCTION: LEGO1 0x10036420
// FUNCTION: BETA10 0x10022b84
MxLong Ambulance::Notify(MxParam& p_param)
{
	MxLong result = 0;
	MxNotificationParam& param = (MxNotificationParam&) p_param;

	switch (param.GetNotification()) {
	case c_notificationType0:
		result = HandleNotification0();
		break;
	case c_notificationEndAction:
		result = HandleEndAction((MxEndActionNotificationParam&) p_param);
		break;
	case c_notificationButtonDown:
		result = HandleButtonDown((LegoControlManagerNotificationParam&) p_param);
		break;
	case c_notificationClick:
		result = HandleClick();
		break;
	case c_notificationControl:
		result = HandleControl((LegoControlManagerNotificationParam&) p_param);
		break;
	case c_notificationPathStruct:
		result = HandlePathStruct((LegoPathStructNotificationParam&) p_param);
		break;
	}

	return result;
}

// FUNCTION: LEGO1 0x100364d0
// FUNCTION: BETA10 0x10022cc2
MxLong Ambulance::HandleEndAction(MxEndActionNotificationParam& p_param)
{
	if (p_param.GetAction() != NULL) {
		IsleScript::Script objectId = (IsleScript::Script) p_param.GetAction()->GetObjectId();

		if (m_lastAnimation == objectId) {
			m_lastAnimation = IsleScript::c_noneIsle;
		}

		if (m_lastAction == objectId) {
			if (m_lastAnimation == IsleScript::c_noneIsle) {
				BackgroundAudioManager()->RaiseVolume();
			}

			m_lastAction = IsleScript::c_noneIsle;
		}
		else if (objectId == IsleScript::c_hho027en_RunAnim) {
			m_state->m_state = AmbulanceMissionState::e_enteredAmbulance;
			CurrentWorld()->PlaceActor(UserActor());
			HandleClick();
			m_enableRandomAudio = 0;
			TickleManager()->RegisterClient(this, 40000);
		}
		else if (objectId == IsleScript::c_hpz047pe_RunAnim || objectId == IsleScript::c_hpz048pe_RunAnim || objectId == IsleScript::c_hpz049bd_RunAnim || objectId == IsleScript::c_hpz053pa_RunAnim) {
			if (m_taskState == Ambulance::e_finished) {
				PlayAnimation(IsleScript::c_hpz055pa_RunAnim);
				m_taskState = Ambulance::e_none;
			}
			else {
				PlayAnimation(IsleScript::c_hpz053pa_RunAnim);
			}
		}
		else if (objectId == IsleScript::c_hpz050bd_RunAnim || objectId == IsleScript::c_hpz052ma_RunAnim) {
			if (m_taskState == Ambulance::e_finished) {
				PlayAnimation(IsleScript::c_hpz057ma_RunAnim);
				m_taskState = Ambulance::e_none;
			}
			else {
				PlayAnimation(IsleScript::c_hpz052ma_RunAnim);
			}
		}
		else if (objectId == IsleScript::c_hpz055pa_RunAnim || objectId == IsleScript::c_hpz057ma_RunAnim) {
			CurrentWorld()->PlaceActor(UserActor());
			HandleClick();
			SpawnPlayer(LegoGameState::e_pizzeriaExterior, TRUE, 0);
			m_enableRandomAudio = 0;
			TickleManager()->RegisterClient(this, 40000);

			if (m_atPoliceTask != 0) {
				StopActions();
			}
		}
		else if (objectId == IsleScript::c_hps116bd_RunAnim || objectId == IsleScript::c_hps118re_RunAnim) {
			if (objectId == IsleScript::c_hps116bd_RunAnim && m_taskState != Ambulance::e_finished) {
				PlayAction(IsleScript::c_Avo923In_PlayWav);
			}

			if (m_taskState == Ambulance::e_finished) {
				PlayAnimation(IsleScript::c_hps117bd_RunAnim);
				m_taskState = Ambulance::e_none;
			}
			else {
				PlayAnimation(IsleScript::c_hps118re_RunAnim);
			}
		}
		else if (objectId == IsleScript::c_hps117bd_RunAnim) {
			CurrentWorld()->PlaceActor(UserActor());
			HandleClick();
			SpawnPlayer(LegoGameState::e_policeExited, TRUE, 0);
			m_enableRandomAudio = 0;
			TickleManager()->RegisterClient(this, 40000);

			if (m_atBeachTask != 0) {
				StopActions();
			}
		}
		else if (objectId == IsleScript::c_hho142cl_RunAnim || objectId == IsleScript::c_hho143cl_RunAnim || objectId == IsleScript::c_hho144cl_RunAnim) {
			Reset();
		}
	}

	return 1;
}

// FUNCTION: LEGO1 0x100367c0
// FUNCTION: BETA10 0x100230bf
MxLong Ambulance::HandleButtonDown(LegoControlManagerNotificationParam& p_param)
{
	if (m_taskState == Ambulance::e_waiting) {
		LegoROI* roi = PickROI(p_param.GetX(), p_param.GetY());

		if (roi != NULL && !SDL_strcasecmp(roi->GetName(), "ps-gate")) {
			m_taskState = Ambulance::e_finished;
			return 1;
		}

		roi = PickRootROI(p_param.GetX(), p_param.GetY());

		if (roi != NULL && !SDL_strcasecmp(roi->GetName(), "gd")) {
			m_taskState = Ambulance::e_finished;
			return 1;
		}
	}

	return 0;
}

// FUNCTION: LEGO1 0x10036860
// FUNCTION: BETA10 0x100231bf
MxLong Ambulance::HandlePathStruct(LegoPathStructNotificationParam& p_param)
{
	// 0x168 corresponds to the path at the gas station
	if (p_param.GetData() == 0x168) {
		m_fuel = 1.0f;
	}

	if (p_param.GetTrigger() == LegoPathStruct::c_camAnim && p_param.GetData() == 0x0b) {
		if (m_atBeachTask != 0) {
			if (m_atPoliceTask != 0) {
				m_state->m_state = AmbulanceMissionState::e_prepareAmbulance;

				if (m_lastAction != IsleScript::c_noneIsle) {
					InvokeAction(Extra::e_stop, *g_isleScript, m_lastAction, NULL);
				}

				Leave();
				MxLong time = Timer()->GetTime() - m_state->m_startTime;

				if (time < 300000) {
					m_state->UpdateScore(LegoState::e_red, m_actorId);
					PlayFinalAnimation(IsleScript::c_hho142cl_RunAnim);
				}
				else if (time < 400000) {
					m_state->UpdateScore(LegoState::e_blue, m_actorId);
					PlayFinalAnimation(IsleScript::c_hho143cl_RunAnim);
				}
				else {
					m_state->UpdateScore(LegoState::e_yellow, m_actorId);
					PlayFinalAnimation(IsleScript::c_hho144cl_RunAnim);
				}

				return 0;
			}

			if (m_atBeachTask != 0) {
				if (m_lastAction != IsleScript::c_noneIsle) {
					InvokeAction(Extra::e_stop, *g_isleScript, m_lastAction, NULL);
				}

				PlayAction(IsleScript::c_Avo916In_PlayWav);
				return 0;
			}
		}

		if (m_atPoliceTask != 0) {
			if (m_lastAction != IsleScript::c_noneIsle) {
				InvokeAction(Extra::e_stop, *g_isleScript, m_lastAction, NULL);
			}

			PlayAction(IsleScript::c_Avo915In_PlayWav);
		}
	}
	else if (p_param.GetTrigger() == LegoPathStruct::c_s && p_param.GetData() == 0x131 && m_atBeachTask == 0) {
		m_atBeachTask = 1;
		m_taskState = Ambulance::e_waiting;

		if (m_lastAction != IsleScript::c_noneIsle) {
			InvokeAction(Extra::e_stop, *g_isleScript, m_lastAction, NULL);
		}

		Leave();

		if (m_actorId < LegoActor::c_pepper || m_actorId > LegoActor::c_laura) {
			m_actorId = LegoActor::c_laura;
		}

		switch (m_actorId) {
		case c_pepper:
			PlayAnimation(IsleScript::c_hpz049bd_RunAnim);
			break;
		case c_mama:
			PlayAnimation(IsleScript::c_hpz047pe_RunAnim);
			break;
		case c_papa:
			PlayAnimation(IsleScript::c_hpz050bd_RunAnim);
			break;
		case c_nick:
		case c_laura:
			PlayAnimation(IsleScript::c_hpz048pe_RunAnim);
			break;
		}
	}
	else if (p_param.GetTrigger() == LegoPathStruct::c_camAnim && (p_param.GetData() == 0x22 || p_param.GetData() == 0x23 || p_param.GetData() == 0x24) && m_atPoliceTask == 0) {
		m_atPoliceTask = 1;
		m_taskState = Ambulance::e_waiting;

		if (m_lastAction != IsleScript::c_noneIsle) {
			InvokeAction(Extra::e_stop, *g_isleScript, m_lastAction, NULL);
		}

		Leave();
		PlayAnimation(IsleScript::c_hps116bd_RunAnim);
	}

	return 0;
}

// FUNCTION: LEGO1 0x10036ce0
// FUNCTION: BETA10 0x10023506
MxLong Ambulance::HandleClick()
{
	if (((Act1State*) GameState()->GetState("Act1State"))->m_state != Act1State::e_ambulance) {
		return 1;
	}

	if (m_state->m_state == AmbulanceMissionState::e_prepareAmbulance) {
		return 1;
	}

	Disable(TRUE, 0);
	((Isle*) CurrentWorld())->SetDestLocation(LegoGameState::e_ambulance);
	TransitionManager()->StartTransition(MxTransitionManager::e_mosaic, 50, FALSE, FALSE);

	if (UserActor()->GetActorId() != GameState()->GetActorId()) {
		((IslePathActor*) UserActor())->Exit();
	}

	m_time = Timer()->GetTime();
	m_actorId = UserActor()->GetActorId();

	Enter();
	InvokeAction(Extra::e_start, *g_isleScript, IsleScript::c_AmbulanceDashboard, NULL);
	ControlManager()->Register(this);

	if (m_state->m_state == AmbulanceMissionState::e_enteredAmbulance) {
		SpawnPlayer(LegoGameState::e_hospitalExited, TRUE, 0);
		m_state->m_startTime = Timer()->GetTime();
		InvokeAction(Extra::e_start, *g_isleScript, IsleScript::c_pns018rd_RunAnim, NULL);
	}

	return 1;
}

// FUNCTION: LEGO1 0x10036e60
// FUNCTION: BETA10 0x100236bb
void Ambulance::Init()
{
	m_state->m_state = AmbulanceMissionState::e_prepareAmbulance;
	PlayAnimation(IsleScript::c_hho027en_RunAnim);
	m_lastAction = IsleScript::c_noneIsle;
	m_lastAnimation = IsleScript::c_noneIsle;
}

// FUNCTION: LEGO1 0x10036e90
void Ambulance::Exit()
{
	GameState()->m_currentArea = LegoGameState::e_hospitalExterior;
	StopActions();
	Reset();
	Leave();
}

// FUNCTION: LEGO1 0x10036ec0
void Ambulance::Leave()
{
	IslePathActor::Exit();
	CurrentWorld()->RemoveActor(this);
	m_roi->SetVisibility(FALSE);
	RemoveFromCurrentWorld(*g_isleScript, IsleScript::c_AmbulanceDashboard_Bitmap);
	RemoveFromCurrentWorld(*g_isleScript, IsleScript::c_AmbulanceArms_Ctl);
	RemoveFromCurrentWorld(*g_isleScript, IsleScript::c_AmbulanceHorn_Ctl);
	RemoveFromCurrentWorld(*g_isleScript, IsleScript::c_AmbulanceHorn_Sound);
	RemoveFromCurrentWorld(*g_isleScript, IsleScript::c_AmbulanceInfo_Ctl);
	RemoveFromCurrentWorld(*g_isleScript, IsleScript::c_AmbulanceSpeedMeter);
	RemoveFromCurrentWorld(*g_isleScript, IsleScript::c_AmbulanceFuelMeter);
	ControlManager()->Unregister(this);
	TickleManager()->UnregisterClient(this);
}

// FUNCTION: LEGO1 0x10036f90
MxLong Ambulance::HandleControl(LegoControlManagerNotificationParam& p_param)
{
	MxLong result = 0;

	if (p_param.m_enabledChild == 1) {
		switch (p_param.m_clickedObjectId) {
		case IsleScript::c_AmbulanceArms_Ctl:
			Exit();
			GameState()->m_currentArea = LegoGameState::e_vehicleExited;
			result = 1;
			break;
		case IsleScript::c_AmbulanceInfo_Ctl:
			((Isle*) CurrentWorld())->SetDestLocation(LegoGameState::e_infomain);
			TransitionManager()->StartTransition(MxTransitionManager::e_mosaic, 50, FALSE, FALSE);
			Exit();
			GameState()->m_currentArea = LegoGameState::e_vehicleExited;
			result = 1;
			break;
		case IsleScript::c_AmbulanceHorn_Ctl:
			MxSoundPresenter* presenter =
				(MxSoundPresenter*) CurrentWorld()->Find("MxSoundPresenter", "AmbulanceHorn_Sound");
			presenter->Enable(p_param.m_enabledChild);
			break;
		}
	}

	return result;
}

// FUNCTION: LEGO1 0x10037060
void Ambulance::ActivateSceneActions()
{
	PlayMusic(JukeboxScript::c_Hospital_Music);

	if (m_state->m_state == AmbulanceMissionState::e_enteredAmbulance) {
		m_state->m_state = AmbulanceMissionState::e_ready;
		PlayAction(IsleScript::c_ham033cl_PlayWav);
	}
	else if (m_atPoliceTask != 0 && m_atBeachTask != 0) {
		IsleScript::Script objectId;

		switch (SDL_rand(2)) {
		case 0:
			objectId = IsleScript::c_ham076cl_PlayWav;
			break;
		case 1:
			objectId = IsleScript::c_ham088cl_PlayWav;
			break;
		}

		if (m_lastAction != IsleScript::c_noneIsle) {
			InvokeAction(Extra::e_stop, *g_isleScript, m_lastAction, NULL);
		}

		PlayAction(objectId);
	}
	else {
		IsleScript::Script objectId;

		switch (SDL_rand(2)) {
		case 0:
			objectId = IsleScript::c_ham075cl_PlayWav;
			break;
		case 1:
			objectId = IsleScript::c_ham113cl_PlayWav;
			break;
		}

		if (m_lastAction != IsleScript::c_noneIsle) {
			InvokeAction(Extra::e_stop, *g_isleScript, m_lastAction, NULL);
		}

		PlayAction(objectId);
	}
}

// FUNCTION: LEGO1 0x10037160
// FUNCTION: BETA10 0x100237df
MxResult Ambulance::Tickle()
{
	if (m_enableRandomAudio == 0) {
		m_enableRandomAudio = 1;
	}
	else if (m_lastAction == IsleScript::c_noneIsle) {
		IsleScript::Script objectId;

		switch (1 + SDL_rand(12)) {
		case 1:
			objectId = IsleScript::c_ham034ra_PlayWav;
			break;
		case 2:
			objectId = IsleScript::c_ham035ra_PlayWav;
			break;
		case 3:
			objectId = IsleScript::c_ham036ra_PlayWav;
			break;
		case 4:
			objectId = IsleScript::c_hpz037ma_PlayWav;
			break;
		case 5:
			objectId = IsleScript::c_sns078pa_PlayWav;
			break;
		case 6:
			objectId = IsleScript::c_ham039ra_PlayWav;
			break;
		case 7:
			objectId = IsleScript::c_ham040cl_PlayWav;
			break;
		case 8:
			objectId = IsleScript::c_ham041cl_PlayWav;
			break;
		case 9:
			objectId = IsleScript::c_ham042cl_PlayWav;
			break;
		case 10:
			objectId = IsleScript::c_ham043cl_PlayWav;
			break;
		case 11:
			objectId = IsleScript::c_ham044cl_PlayWav;
			break;
		case 12:
			objectId = IsleScript::c_ham045cl_PlayWav;
			break;
		}

		PlayAction(objectId);
	}

	return SUCCESS;
}

// FUNCTION: LEGO1 0x10037240
void Ambulance::StopActions()
{
	StopAction(IsleScript::c_pns018rd_RunAnim);
}

// FUNCTION: LEGO1 0x10037250
void Ambulance::Reset()
{
	StopAction(m_lastAction);
	BackgroundAudioManager()->RaiseVolume();
	((Act1State*) GameState()->GetState("Act1State"))->m_state = Act1State::e_none;
	m_state->m_state = AmbulanceMissionState::e_ready;
	m_atBeachTask = 0;
	m_atPoliceTask = 0;
	g_isleFlags |= Isle::c_playMusic;
	AnimationManager()->EnableCamAnims(TRUE);
	AnimationManager()->FUN_1005f6d0(TRUE);
	m_state->m_startTime = INT_MIN;
	m_state = NULL;
}

// FUNCTION: LEGO1 0x100372e0
// FUNCTION: BETA10 0x100241a0
void Ambulance::PlayAnimation(IsleScript::Script p_objectId)
{
	AnimationManager()
		->FUN_10060dc0(p_objectId, NULL, TRUE, LegoAnimationManager::e_unk0, NULL, FALSE, FALSE, FALSE, TRUE);
	m_lastAnimation = p_objectId;
}

// FUNCTION: LEGO1 0x10037310
// FUNCTION: BETA10 0x10024440
void Ambulance::PlayFinalAnimation(IsleScript::Script p_objectId)
{
	AnimationManager()
		->FUN_10060dc0(p_objectId, NULL, TRUE, LegoAnimationManager::e_unk1, NULL, FALSE, FALSE, TRUE, TRUE);
	m_lastAnimation = p_objectId;
}

// FUNCTION: LEGO1 0x10037340
void Ambulance::StopAction(IsleScript::Script p_objectId)
{
	if (p_objectId != -1) {
		InvokeAction(Extra::e_stop, *g_isleScript, p_objectId, NULL);
	}
}

// FUNCTION: LEGO1 0x10037360
void Ambulance::PlayAction(IsleScript::Script p_objectId)
{
	if (p_objectId != -1) {
		InvokeAction(Extra::e_start, *g_isleScript, p_objectId, NULL);
	}

	m_lastAction = p_objectId;
	BackgroundAudioManager()->LowerVolume();
}

// FUNCTION: LEGO1 0x100373a0
AmbulanceMissionState::AmbulanceMissionState()
{
	m_state = AmbulanceMissionState::e_ready;
	m_startTime = 0;
	m_peScore = 0;
	m_maScore = 0;
	m_paScore = 0;
	m_niScore = 0;
	m_laScore = 0;
	m_peHighScore = 0;
	m_maHighScore = 0;
	m_paHighScore = 0;
	m_niHighScore = 0;
	m_laHighScore = 0;
}
