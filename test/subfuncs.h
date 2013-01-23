/* DoSound */
#if SCI_VERSION < SCI_VERSION_1_EARLY

#define f_DoSound_Init					0
#define f_DoSound_Play					1
#define f_DoSound_Restore				2
#define f_DoSound_Dispose				3
#define f_DoSound_Mute					4
#define f_DoSound_Stop					5
#define f_DoSound_Pause					6
#define f_DoSound_ResumeAfterRestore	7
#define f_DoSound_MasterVolume			8
#define f_DoSound_Update				9
#define f_DoSound_Fade					10
#define f_DoSound_GetPolyphony			11
#define f_DoSound_StopAll				12

#elif SCI_VERSION < SCI_VERSION_1_LATE

#define f_DoSound_MasterVolume			0
#define f_DoSound_Mute					1
#define f_DoSound_Restore				2
#define f_DoSound_GetPolyphony			3
#define f_DoSound_Update				4
#define f_DoSound_Init					5
#define f_DoSound_Dispose				6
#define f_DoSound_Play					7
#define f_DoSound_Stop					8
#define f_DoSound_Pause					9
#define f_DoSound_Fade					10
#define f_DoSound_UpdateCues			11
#define f_DoSound_SendMidi				12
#define f_DoSound_GlobalReverb			13
#define f_DoSound_SetHold				14

#else

#define f_DoSound_MasterVolume			0
#define f_DoSound_Mute					1
#define f_DoSound_Restore				2
#define f_DoSound_GetPolyphony			3
#define f_DoSound_GetAudioCapability	4
#define f_DoSound_Suspend				5
#define f_DoSound_Init					6
#define f_DoSound_Dispose				7
#define f_DoSound_Play					8
#define f_DoSound_Stop					9
#define f_DoSound_Pause					10
#define f_DoSound_Fade					11
#define f_DoSound_SetHold				12
#define f_DoSound_SetVolume				14
#define f_DoSound_SetPriority			15
#define f_DoSound_SetLoop				16
#define f_DoSound_UpdateCues			17
#define f_DoSound_SendMidi				18
#define f_DoSound_GlobalReverb			19
#define f_DoSound_Update				20

#endif

/* Graph */
#define f_Graph_GetColorCount			2
#define f_Graph_DrawLine				4
#define f_Graph_SaveBox					7
#define f_Graph_RestoreBox				8
#define f_Graph_FillBoxBackground		9
#define f_Graph_FillBoxForeground		10
#define f_Graph_FillBoxAny				11
#define f_Graph_UpdateBox				12
#define f_Graph_RedrawBox				13
#define f_Graph_AdjustPriority			14
#define f_Graph_SaveUpscaledHiresBox	15

/* PalVary */
#define f_PalVary_Init					0
#define f_PalVary_Reverse				1
#define f_PalVary_GetCurrenStep			2
#define f_PalVary_Deinit				3
#define f_PalVary_ChangeTarget			4
#define f_PalVary_ChangeTicks			5
#define f_PalVary_PauseResume			6

/* Palette */
#define f_Palette_SetFromResources		1
#define f_Palette_SetFlag				2
#define f_Palette_UnsetFlag				3
#define f_Palette_SetIntensity			4
#define f_Palette_FindColor				5
#define f_Palette_Animate				6
#define f_Palette_Save					7
#define f_Palette_Restore				8

/* FileIO */
#define f_FileIO_Open					0
#define f_FileIO_Close					1
#define f_FileIO_ReadRaw				2
#define f_FileIO_WriteRaw				3
#define f_FileIO_Unlink					4
#define f_FileIO_ReadString				5
#define f_FileIO_WriteString			6
#define f_FileIO_Seek					7
#define f_FileIO_FindFirst				8
#define f_FileIO_FindNext				9
#define f_FileIO_Exists					10
#define f_FileIO_Rename					11
#define f_FileIO_ReadByte				13
#define f_FileIO_WriteByte				14
#define f_FileIO_ReadWord				15
#define f_FileIO_WriteWord				16
#define f_FileIO_CreateSaveSlot			17
#define f_FileIO_ChangeDirectory		18
#define f_FileIO_IsValidDirectory		19

/* Save */
#define f_Save_SaveGame					0
#define f_Save_RestoreGame				1
#define f_Save_GetSaveDir				2
#define f_Save_CheckSaveGame			3
#define f_Save_GetSaveFiles				5
#define f_Save_MakeSaveCatName			6
#define f_Save_MakeSaveFileName			7
#define f_Save_AutoSave					8

/* List */
#define f_List_NewList					0
#define f_List_DisposeList				1
#define f_List_NewNode					2
#define f_List_FirstNode				3
#define f_List_LastNode					4
#define f_List_EmptyList				5
#define f_List_NextNode					6
#define f_List_PrevNode					7
#define f_List_NodeValue				8
#define f_List_AddAfter					9
#define f_List_AddToFront				10
#define f_List_AddToEnd					11
#define f_List_AddBefore				12
#define f_List_MoveToFront				13
#define f_List_MoveToEnd				14
#define f_List_FindKey					15
#define f_List_DeleteKey				16
#define f_List_ListAt					17
#define f_List_ListIndexOf				18
#define f_List_ListEachElementDo		19
#define f_List_ListFirstTrue			20
#define f_List_ListAllTrue				21
#define f_List_Sort						22

/* DeviceInfo */
#define f_DeviceInfo_GetDevice			0
#define f_DeviceInfo_GetCurrentDevice	1
#define f_DeviceInfo_InfoPathsEqual		2
#define f_DeviceInfo_InfoIsFloppy		3
#define f_DeviceInfo_GetConfigPath		5
#define f_DeviceInfo_GetSaveCatName		7
#define f_DeviceInfo_GetSaveFileName	8

/* Memory */
#define f_Memory_AllocateCritical		1
#define f_Memory_AllocateNonCritical	2
#define f_Memory_Free					3
#define f_Memory_MemCpy					4
#define f_Memory_Peek					5
#define f_Memory_Poke					6
