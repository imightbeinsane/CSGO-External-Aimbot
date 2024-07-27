#pragma once
#include "memory.h"
#include "vector.h"
#include <iostream>
#include <thread>

// THIS IS NOT A REAL CS:GO CHEAT IT IS MADE FOR EDUCATION PURPOSES ONLY
// BEFOR LAUNCHING THE GAME GO TO LAUNCH OPTIONS FROM STEAM AND ADD "-insecure"
// IF YOU DONT DO THAT YOU WILL GET BANNED IN JUST A SECOND
// I AM NOT RESPONSIBLE FOR ANYTHING


// Offsets namespace
namespace offset
{
	// Client offsets
	constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDEF97C;
	constexpr ::std::ptrdiff_t dwEntityList = 0x4E051DC;
	constexpr ::std::ptrdiff_t dwForceAttack = 0x3233024;
	constexpr ::std::ptrdiff_t m_iCrosshairId = 0x11838;

	// Engine offsets
	constexpr ::std::ptrdiff_t dwClientState = 0x59F19C;
	constexpr ::std::ptrdiff_t dwClientState_ViewAngles = 0x4D90;
	constexpr ::std::ptrdiff_t dwClientState_GetLocalPlayer = 0x180;

	// Entity Offsets
	constexpr ::std::ptrdiff_t m_dwBoneMatrix = 0x26A8;
	constexpr ::std::ptrdiff_t m_bDormant = 0xED;
	constexpr ::std::ptrdiff_t m_iTeamNum = 0xF4;
	constexpr ::std::ptrdiff_t m_lifeState = 0x25F;
	constexpr ::std::ptrdiff_t m_vecOrigin = 0x138;
	constexpr ::std::ptrdiff_t m_vecViewOffset = 0x108;
	constexpr ::std::ptrdiff_t m_aimPunchAngle = 0x303C;
	constexpr ::std::ptrdiff_t m_bSpottedByMask = 0x980;

	std::uintptr_t localPlayer; // Local player variable
	std::int32_t localTeam;		// Local team variable (Program only updates this value when aimbot starts so if you change team, restart the aimbot)
	float smoothing = 1.f;		// Smoothing value bigger the value is slower the aim
	int boneIndex = 8;			// Bone index of the head in CS:GO if you want to change aimbot to chest or any other bone, change the index
	std::uintptr_t clientState; // Client state pointer to get viewAngles and more
}


CVector3 CalculateAngle(		// Function the calculate the angle to look at
	const CVector3& localPosition,
	const CVector3& enemyPosition,
	const CVector3& viewAngles) noexcept 
{
	return ((enemyPosition - localPosition).ToAngle() - viewAngles); // Calculate angles and return
}

float CalculateDistance(const CVector3& firstPos, const CVector3& secondPos)			// Calculates the distance between two points
{
	CVector3 delta = secondPos - firstPos;		// We are changing the origin to our location to understand that you need a bit math
	float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);	// Calculating distance 
	return distance;	// Return distance
}


CVector3 FindClosestEntity(CVector3& localPos, Memory& mem, std::uintptr_t client)		// Finding the closest entity to look at
{
	CVector3 closestEnemyLocation = CVector3{ 100000.f, 100000.f, 10000.f };			// To hold the closest entity location
	float closestDistance = 10000.f;		// To hold distance so we can compare next entitys distance and decide if its closer or not
	for (auto i = 1; i <= 64; i++)
	{
		const auto player = mem.Read<std::uintptr_t>(client + offset::dwEntityList + i * 0x10);		// The 0x10 is the length of an entity

		// Checking if the player is visible if we dont check we will lock onto enemies behind walls
		// If you want to lock onto enemies behind walls remove this line
		if (player + offset::m_bSpottedByMask == false) continue;

		// If the player is on our team or If the player is dormant (If a player is dormant server doesnt upload its location to us) jump to next entity;
		if (mem.Read<std::int32_t>(player + offset::m_iTeamNum) == offset::localTeam || mem.Read<bool>(player + offset::m_bDormant)) continue;

		// Check if the player is alive 0 means alive others are dead
		if (mem.Read<std::int32_t>(player + offset::m_lifeState)) continue;

		// Get the bones of the player
		const auto boneMatrix = mem.Read<std::uintptr_t>(player + offset::m_dwBoneMatrix);
		
		// Locate the head
		CVector3 playerHeadPosition = CVector3{
			// 0x30 is the length of the boneMatrix
			mem.Read<float>(boneMatrix + 0x30 * offset::boneIndex + 0x0C), // The x axis is in the 0x0C
			mem.Read<float>(boneMatrix + 0x30 * offset::boneIndex + 0x1C), // The y axis is in the 0x1C
			mem.Read<float>(boneMatrix + 0x30 * offset::boneIndex + 0x2C)  // The z axis is in the 0x2C
		};

		// Calculate between our local players location and players position
		float distance = CalculateDistance(localPos, playerHeadPosition);

		// If current players distance is closer than previus onces distance set the closest enemy location and closest distance
		if (distance < closestDistance)
		{
				closestDistance = distance;
				closestEnemyLocation = playerHeadPosition;
		}
	}
	// Retun the location to look at
	return closestEnemyLocation;
}

void TriggerBot(std::uintptr_t client, Memory& memory)		// Triggerbot function
{
	const auto crossId = memory.Read<std::int32_t>(offset::localPlayer + offset::m_iCrosshairId);
	// Get the player we are looking at
	const auto player = memory.Read<std::uintptr_t>(client + offset::dwEntityList + (crossId - 1) * 0x10);
	
	// If we cannot see the player return
	if (!memory.Read<bool>(player + offset::m_bSpottedByMask)) return;

	if (memory.Read<std::int32_t>(player + offset::m_iTeamNum) == offset::localTeam) return;

	// Forcing localPlayer to attack
	memory.Write<std::uintptr_t>(client + offset::dwForceAttack, 6);
	
	// Waiting 20 milliseconds and changing the forceAttack to 4 if we dont change it to 4 we will never be able to attack again
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	memory.Write<std::uintptr_t>(client + offset::dwForceAttack, 4);
}

void LookAt(CVector3& localEyePosition, CVector3 locationToLook, Memory& memory, std::uintptr_t client)		// To look somewhere use that function
{
	// Get viewAngles and the aimPunch and multiply with 2 if you are asking why do some research in the leaked source code
	const CVector3 viewAngles = memory.Read<CVector3>(offset::clientState + offset::dwClientState_ViewAngles);
	const CVector3 aimPunch = memory.Read<CVector3>(offset::localPlayer + offset::m_aimPunchAngle) * 2;

	// Calculating the angle to look at
	const CVector3 bestAngle = CalculateAngle(localEyePosition, locationToLook, viewAngles + aimPunch);

	// Check If we have a valid angle
	if (!bestAngle.IsZero())
		// Write to the memory if we have a valid angle
		memory.Write<CVector3>(offset::clientState + offset::dwClientState_ViewAngles, viewAngles + bestAngle / offset::smoothing);
		// After aiming attack
		TriggerBot(client, memory);
}



int Aimbot(Memory memory, std::uintptr_t client, std::uintptr_t engine)		// Aimbot function
{
	// Infinite loop 
	while (true)
	{
		// If "C" key is not pressed dont do anything you can learn virtual key codes in windows documentary
		if (!GetAsyncKeyState(0x43)) continue;

		// Players eye position is vec origin + vec view offset
		CVector3 localEyePosition = memory.Read<CVector3>(offset::localPlayer + offset::m_vecOrigin) + memory.Read<CVector3>(offset::localPlayer + offset::m_vecViewOffset);

		// Get the client state
		offset::clientState = memory.Read<std::uintptr_t>(engine + offset::dwClientState);

		// Call FindClosestEntity to find the closest entity
		const CVector3 playerHeadPosition = FindClosestEntity(localEyePosition, memory, client);
		
		// We are checking if x axis and y axis is same because the only place they can be same is the origin of the world and we dont want to lock on world origin
		if (playerHeadPosition.x == playerHeadPosition.y) continue;

		// Look at the players head
		LookAt(localEyePosition, playerHeadPosition, memory, client);
		
		// Sleep 1 milliseconds otherwise its going to decrease fps
		// std::this_thread::sleep_for(std::chrono::milliseconds(1));
		
		// If you press "End" key it will exit the program
		if (GetAsyncKeyState(VK_END)) return 0;
	}

}

int main()
{
	// Find csgo.exe
	Memory memory = Memory{ "csgo.exe" };

	// Find "client.dll" and "engine.dll"
	const std::uintptr_t client = memory.GetModuleAddress("client.dll");
	const std::uintptr_t engine = memory.GetModuleAddress("engine.dll");

	// Get local player and get local team
	offset::localPlayer = memory.Read<std::uintptr_t>(client + offset::dwLocalPlayer);
	offset::localTeam = memory.Read<std::int32_t>(offset::localPlayer + offset::m_iTeamNum);
	Aimbot(memory, client, engine);
}