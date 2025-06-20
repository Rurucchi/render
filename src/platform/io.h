/*
	io
*/

#ifndef _IOH_
#define _IOH_


typedef struct complete_file {
	ui32 size;
	void *memory;
} completeFile;

// disable compiler memory alligment
// #pragma pack(push, 1)
// typedef struct file_bitmap_header
// {
    //// header: 14 bytes
    // char formatID[2];
    // ui32 size;
    // ui32 pad;
    // ui32 offset; // start of the data array
// } file_bitmap_header;

// typedef struct file_bitmap_info_header
// {
    // ui32 headerSize;
    // i32 width;
    // i32 height;
    // ui16 numPlanes;
    // ui16 bpp; // bits per pixel
    // char compression[4];
    // ui32 imageSize;
    // i32 hres;
    // i32 vres;
    // ui32 numColors; // number of colors in the palette
    // ui32 numImpColors; // number of important colors used
// } file_bitmap_info_header;
// #pragma pack(pop)

//// texture
// typedef struct texture {
	// void *memory;
	// int memorySize;
	// int width;
	// int height;
// } texture;


HANDLE io_create_handle(char *location) {
	HANDLE rawFile = CreateFileA(location, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	
	if (rawFile == INVALID_HANDLE_VALUE)
    {
		OutputDebugStringA("FILE OPENING ERROR\n");
	}
	
	return rawFile;
}

void io_file_fullread(char *location, complete_file *file){
	HANDLE rawFile = io_create_handle(location);
	
	//check handle
    if (rawFile == INVALID_HANDLE_VALUE)
    {
		OutputDebugStringA("FILE OPENING ERROR\n");
    } else {
		LARGE_INTEGER fileSize;
	
		GetFileSizeEx(rawFile,
		  &fileSize
		);
	
		ui32 fileSize32 = SafeTruncateUInt64(fileSize.QuadPart);
		file->memory = VirtualAlloc(0, fileSize32, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		ReadFile(rawFile, file->memory, fileSize32, NULL, NULL);
		file->size = fileSize32;
	}

}

void io_file_fullfree(complete_file *file){
	VirtualFree(
	  file->memory,
	  file->size,
	  MEM_RELEASE
	);
}

#endif /* _IOH_ */