#include "legodxinfo.h"

#include <SDL3/SDL_cpuinfo.h>
#include <assert.h>
#include <stdio.h> // for vsprintf

// File name validated by BETA10 0x1011cba3; directory unknown

// FUNCTION: CONFIG 0x00402560
// FUNCTION: LEGO1 0x1009ce60
// FUNCTION: BETA10 0x1011c7e0
int LegoDeviceEnumerate::ParseDeviceName(const char* p_deviceId)
{
	if (!IsInitialized()) {
		return -1;
	}

	int unknown = -1;
	int num = -1;
	int hex[4];

	if (sscanf(p_deviceId, "%d 0x%x 0x%x 0x%x 0x%x", &num, &hex[0], &hex[1], &hex[2], &hex[3]) != 5) {
		return -1;
	}

	if (num < 0) {
		return -1;
	}

	GUID guid;
	memcpy(&guid, hex, sizeof(guid));

	int result = ProcessDeviceBytes(num, guid);

	if (result < 0) {
		result = ProcessDeviceBytes(-1, guid);
	}

	return result;
}

// FUNCTION: CONFIG 0x00402620
// FUNCTION: LEGO1 0x1009cf20
// FUNCTION: BETA10 0x1011c8b3
int LegoDeviceEnumerate::ProcessDeviceBytes(int p_deviceNum, GUID& p_guid)
{
	if (!IsInitialized()) {
		return -1;
	}

	int i = 0;
	int j = 0;

	static_assert(sizeof(GUID4) == sizeof(GUID), "Equal size");

	GUID4 deviceGuid;
	memcpy(&deviceGuid, &p_guid, sizeof(GUID4));

	for (list<MxDriver>::iterator it = m_ddInfo.begin(); it != m_ddInfo.end(); it++, i++) {
		if (p_deviceNum >= 0 && p_deviceNum < i) {
			return -1;
		}

		GUID4 compareGuid;
		MxDriver& driver = *it;
		for (list<Direct3DDeviceInfo>::iterator it2 = driver.m_devices.begin(); it2 != driver.m_devices.end(); it2++) {
			Direct3DDeviceInfo& md3d = *it2;
			assert(md3d.m_guid);

			memcpy(&compareGuid, md3d.m_guid, sizeof(GUID4));

			if (GUID4::Compare(compareGuid, deviceGuid) && i == p_deviceNum) {
				return j;
			}

			j++;
		}
	}

	return -1;
}

// FUNCTION: CONFIG 0x00402730
// FUNCTION: LEGO1 0x1009d030
// FUNCTION: BETA10 0x1011ca54
int LegoDeviceEnumerate::GetDevice(int p_deviceNum, MxDriver*& p_driver, Direct3DDeviceInfo*& p_device)
{
	if (p_deviceNum < 0 || !IsInitialized()) {
		return -1;
	}

	int i = 0;

	for (list<MxDriver>::iterator it = m_ddInfo.begin(); it != m_ddInfo.end(); it++) {
		p_driver = &*it;

		for (list<Direct3DDeviceInfo>::iterator it2 = p_driver->m_devices.begin(); it2 != p_driver->m_devices.end();
			 it2++) {
			if (i == p_deviceNum) {
				p_device = &*it2;
				return 0;
			}
			i++;
		}
	}

	return -1;
}

// FUNCTION: CONFIG 0x004027d0
// FUNCTION: BETA10 0x1011cb70
int LegoDeviceEnumerate::FormatDeviceName(char* p_buffer, const MxDriver* p_ddInfo, const Direct3DDeviceInfo* p_d3dInfo)
	const
{
	int number = 0;
	assert(p_ddInfo && p_d3dInfo);

	for (list<MxDriver>::const_iterator it = m_ddInfo.begin(); it != m_ddInfo.end(); it++, number++) {
		if (&(*it) == p_ddInfo) {
			GUID4 guid;
			memcpy(&guid, p_d3dInfo->m_guid, sizeof(GUID4));

			sprintf(p_buffer, "%d 0x%x 0x%x 0x%x 0x%x", number, guid.m_data1, guid.m_data2, guid.m_data3, guid.m_data4);
			return 0;
		}
	}

	return -1;
}

// FUNCTION: BETA10 0x1011cc65
int LegoDeviceEnumerate::BETA_1011cc65(int p_idx, char* p_buffer)
{
	if (p_idx < 0 || !IsInitialized()) {
		return -1;
	}

	int i = 0;
	int j = 0;

	for (list<MxDriver>::iterator it = m_ddInfo.begin(); it != m_ddInfo.end(); it++, i++) {
		MxDriver& driver = *it;
		for (list<Direct3DDeviceInfo>::iterator it2 = driver.m_devices.begin(); it2 != driver.m_devices.end(); it2++) {

			if (j == p_idx) {
				GUID4 guid;
				memcpy(&guid, &((Direct3DDeviceInfo&) *it2).m_guid, sizeof(GUID4));
				sprintf(p_buffer, "%d 0x%x 0x%x 0x%x 0x%x", i, guid.m_data1, guid.m_data2, guid.m_data3, guid.m_data4);
				return 0;
			}

			j++;
		}
	}

	return -1;
}

// FUNCTION: CONFIG 0x00402860
// FUNCTION: LEGO1 0x1009d0d0
// FUNCTION: BETA10 0x1011cdb4
int LegoDeviceEnumerate::GetBestDevice()
{
	if (!IsInitialized()) {
		return -1;
	}

	if (m_ddInfo.size() == 0) {
		return -1;
	}

	int j = 0;
	int k = -1;

	for (list<MxDriver>::iterator it = m_ddInfo.begin(); it != m_ddInfo.end(); it++) {

		MxDriver& driver = *it;
		for (list<Direct3DDeviceInfo>::iterator it2 = driver.m_devices.begin(); it2 != driver.m_devices.end(); it2++) {
			if ((*it2).m_HWDesc.dcmColorModel != D3DCOLOR_NONE) {
				return j;
			}
			else if ((*it2).m_HELDesc.dcmColorModel != D3DCOLOR_NONE) {
				k = j;
			}

			j++;
		}
	}

	return k;
}

// FUNCTION: CONFIG 0x004029a0
// FUNCTION: LEGO1 0x1009d210
// FUNCTION: BETA10 0x1011cfc4
int LegoDeviceEnumerate::FUN_1009d210()
{
	if (!IsInitialized()) {
		return -1;
	}

	for (list<MxDriver>::iterator it = m_ddInfo.begin(); it != m_ddInfo.end();) {
		MxDriver& driver = *it;

		for (list<Direct3DDeviceInfo>::iterator it2 = driver.m_devices.begin(); it2 != driver.m_devices.end();) {
			if (!FUN_1009d3d0(*it2)) {
				driver.m_devices.erase(it2++);
			}
			else {
				it2++;
			}
		}

		if (!driver.m_devices.size()) {
			m_ddInfo.erase(it++);
		}
		else {
			it++;
		}
	}

	if (!m_ddInfo.size()) {
		return -1;
	}

	return 0;
}

// FUNCTION: CONFIG 0x00402b60
// FUNCTION: LEGO1 0x1009d3d0
// FUNCTION: BETA10 0x1011d235
unsigned char LegoDeviceEnumerate::FUN_1009d3d0(Direct3DDeviceInfo& p_device)
{
	if (m_ddInfo.size() <= 0) {
		return FALSE;
	}

	if (p_device.m_HWDesc.dcmColorModel != D3DCOLOR_NONE) {
		DDBitDepths zDepth = p_device.m_HWDesc.dwDeviceZBufferBitDepth;
		if ((zDepth & (DDBD_16 | DDBD_24 | DDBD_32)) != static_cast<DDBitDepths>(0) &&
			(p_device.m_HWDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE) == D3DPTEXTURECAPS_PERSPECTIVE) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}

	MxDriver& front = m_ddInfo.front();
	for (list<Direct3DDeviceInfo>::iterator it = front.m_devices.begin(); it != front.m_devices.end(); it++) {
		if ((&*it) == &p_device) {
			return TRUE;
		}
	}

	return FALSE;
}
