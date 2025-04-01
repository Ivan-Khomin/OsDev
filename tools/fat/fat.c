#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

typedef struct
{
    // BIOS Boot Parameters
    uint8_t bootJumpInstruction[3];
    uint8_t oem[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t fatCount;
    uint16_t rootEntryCount;
    uint16_t totalSectors;
    uint8_t mediaDescType;
    uint16_t sectorsPerFat;
    uint16_t sectorsPerTrack;
    uint16_t heads;
    uint32_t hiddenSectors;
    uint32_t largeSectorCount;

    // Extended Boot Record
    uint8_t driveNumber;
    uint8_t reserved;
    uint8_t signature;
    uint32_t volumeID;
    uint8_t volumeLabel[11];
    uint8_t fileSystem[8];
} __attribute((packed)) BootSector;

typedef struct
{
    uint8_t name[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creationTimeTenth;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t accessedDate;
    uint16_t firstClusterHigh;
    uint16_t lastModificationTime;
    uint16_t lastModificationDate;
    uint16_t firstClusterLow;
    uint32_t size;
} __attribute((packed)) DirectoryEntry;

BootSector bootSector;
uint8_t* fat = NULL;
DirectoryEntry* rootDirectory = NULL;
uint32_t rootDirectoryEnd = 0;

bool readBootSector(FILE* disk);

bool readSectors(FILE* disk, uint32_t lba, uint32_t count, void* buffer);

bool readFat(FILE* disk);

bool readRootDirectory(FILE* disk);

DirectoryEntry* findFile(const char* name);

bool readFile(FILE* disk, DirectoryEntry* fileEntry, uint8_t* buffer);

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("Syntax: %s <disk_image> <filename>\n", argv[0]);
        return -1;
    }
    FILE* disk = fopen(argv[1], "rb");

    if (!disk)
    {
        fprintf(stderr, "Cannot open disk image %s!\n", argv[1]);
        return -1;
    }

    if (!readBootSector(disk))
    {
        fprintf(stderr, "Could not read boot sector!\n");
        return -2;
    }

    if (!readFat(disk))
    {
        fprintf(stderr, "Could not read FAT!\n");
        free(fat);
        return -3;
    }

    if (!readRootDirectory(disk))
    {
        fprintf(stderr, "Could not read root directory!\n");
        free(fat);
        free(rootDirectory);
        return -4;
    }

    DirectoryEntry* fileEntry = findFile(argv[2]);

    if (!fileEntry)
    {
        fprintf(stderr, "Could not find file %s!\n", argv[2]);
        free(fat);
        free(rootDirectory);
        return -5;
    }

    uint8_t* buffer = (uint8_t*) malloc(fileEntry->size + bootSector.bytesPerSector);
    if (!readFile(disk, fileEntry, buffer))
    {
        fprintf(stderr, "Could not read file %s!\n", argv[2]);
        free(fat);
        free(rootDirectory);
        free(buffer);
        return -5;
    }

    for (size_t i = 0; i < fileEntry->size; i++)
    {
        if (isprint(buffer[i])) fputc(buffer[i], stdout);
        else printf("<%02x>", buffer[i]);
    }
    printf("\n");

    free(fat);
    free(rootDirectory);
    free(buffer);
    
    return 0;
}

bool readBootSector(FILE* disk)
{
    return fread(&bootSector, sizeof(bootSector), 1, disk) > 0;
}

bool readSectors(FILE* disk, uint32_t lba, uint32_t count, void* buffer)
{
    bool ok = true;
    ok = ok && (fseek(disk, lba * bootSector.bytesPerSector, SEEK_SET) == 0);
    ok = ok && (fread(buffer, bootSector.bytesPerSector, count, disk) == count);
    return ok;
}

bool readFat(FILE* disk)
{
    fat = (uint8_t*) malloc(bootSector.bytesPerSector * bootSector.sectorsPerFat);
    return readSectors(disk, bootSector.reservedSectors, bootSector.sectorsPerFat, fat);
}

bool readRootDirectory(FILE* disk)
{
    uint32_t lba = bootSector.reservedSectors + bootSector.sectorsPerFat * bootSector.fatCount;
    uint32_t size = sizeof(DirectoryEntry) * bootSector.rootEntryCount;
    uint32_t sectors = size / bootSector.bytesPerSector;

    if (size % bootSector.bytesPerSector > 0)
        sectors++;

    rootDirectoryEnd = lba + sectors;
    rootDirectory = (DirectoryEntry*) malloc(sizeof(DirectoryEntry) * bootSector.rootEntryCount);
    return readSectors(disk, lba, sectors, rootDirectory);
}

DirectoryEntry* findFile(const char* name)
{
    for (size_t i = 0; i < bootSector.rootEntryCount; i++)
        if (memcmp(name, rootDirectory[i].name, 11) == 0)
            return &rootDirectory[i];

    return NULL;
}

bool readFile(FILE* disk, DirectoryEntry* fileEntry, uint8_t* buffer)
{
    bool ok = true;
    uint16_t currentCluster = fileEntry->firstClusterLow;

    do
    {
        uint32_t lba = rootDirectoryEnd + (currentCluster - 2) * bootSector.sectorsPerCluster;
        ok = ok & readSectors(disk, lba, bootSector.sectorsPerCluster, buffer);
        buffer += bootSector.sectorsPerCluster * bootSector.bytesPerSector;

        uint32_t fatIndex = currentCluster * 3 / 2;
        if ((currentCluster & 1) == 0)
            currentCluster = (*(uint16_t*)(fat + fatIndex)) & 0xFFF;
        else
            currentCluster = (*(uint16_t*)(fat + fatIndex)) >> 4;

    } while (ok && currentCluster < 0xFF8);

    return ok;
}