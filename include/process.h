#ifndef PROCESS_H
#define PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#define PressTimoutInterval     4000  // in mS
#define CheckTouchInterval        15  // in mS
#define CheckSwipeInterval        60  // in mS
#define ScreenUpdateInterval     200  // in mS
#define PressTimoutInterval     4000  // in mS

void MakeScreen_Bitmap_RGB(uint32_t FlashAddressddress, uint32_t RAMAddress, uint32_t size);
void MakeScreen_Bitmap_JPEG(uint32_t FlashAddress, uint32_t RAMAddress, uint32_t size);

bool FlashLoad(char* filename );
bool FileTransfer2Flash(char *filename, uint32_t FlashAddress);
bool FlashAttach(void);
bool FlashDetach(void);
bool FlashFast(void);
bool FlashErase(void);


#ifdef __cplusplus
}
#endif

#endif
