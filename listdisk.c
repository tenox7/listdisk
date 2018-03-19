//
// List and Query Physical Disk and Partition Properties on Windows NT based systes
// Copyright (c) 2017-2018 by Antoni Sawicki
//
// v3.3, as@tenoware.com
//

#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <initguid.h>
#include <winternl.h>
#include <Ntddscsi.h>
#include <ntdddisk.h> 
#include <diskguid.h>

#define DIRECTORY_QUERY                 (0x0001)
#define DIRECTORY_TRAVERSE              (0x0002)

typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "ole32.lib")

VOID ListDisk(void);
VOID QueryDisk(WCHAR *);

VOID
ListDisk(
    void
    )
/*++

Routine Description:

    List Physical Disks in Windows NT System by querying NT Namespace for PhysicalDrive* object

--*/
{
    HANDLE hDir;
    OBJECT_ATTRIBUTES attr={0};
    UNICODE_STRING root={0};
    OBJECT_DIRECTORY_INFORMATION dirinfo[1000]={0};
    ULONG i=0, index=0, bytes=0, istart=0, first=0;
    NTSTATUS ret;

    RtlInitUnicodeString(&root, L"\\GLOBAL??");
    InitializeObjectAttributes(&attr, &root, 0, NULL, NULL);

    ret=NtOpenDirectoryObject(&hDir, DIRECTORY_QUERY|DIRECTORY_TRAVERSE, &attr);
    if(ret!=0)
        return;

    first=TRUE;
    istart=0;
    do {
        ret=NtQueryDirectoryObject(hDir, &dirinfo, sizeof(dirinfo),  FALSE, first, &index, &bytes);
        if(ret<0)
            break;

        for(i=0;i<index-istart;i++) 
            if(wcsncmp(dirinfo[i].Name.Buffer, L"PhysicalDrive", wcslen(L"PhysicalDrive"))==0) 
                //wprintf(L"\\\\.\\%s\n", dirinfo[i].Name.Buffer);
                QueryDisk(dirinfo[i].Name.Buffer);

        istart=index;
        first=FALSE;
    } while(TRUE);

    NtClose(hDir);
}


VOID
QueryDisk(
    WCHAR *name
    )
/*++

Routine Description:

    Query Physical Disk for Properties via IOCTL
    Input: (WCHAR*) "PhysicalDiskXX"

--*/
{
    HANDLE hDisk;
    GET_LENGTH_INFORMATION  DiskLengthInfo;
    GET_DISK_ATTRIBUTES DiskAttributes;
    DRIVE_LAYOUT_INFORMATION_EX DiskLayout[1024];
    SCSI_ADDRESS DiskAddress;
    STORAGE_PROPERTY_QUERY desc_q = { StorageDeviceProperty,  PropertyStandardQuery };
    STORAGE_DESCRIPTOR_HEADER desc_h = { 0 };
    PSTORAGE_DEVICE_DESCRIPTOR desc_d;
    STORAGE_PROPERTY_QUERY trim_q = { StorageDeviceTrimProperty,  PropertyStandardQuery };
    DEVICE_TRIM_DESCRIPTOR trim_d = { 0 };
    WCHAR *ft[] = { L"False", L"True" };
    WCHAR *bus[] = { L"UNKNOWN", L"SCSI", L"ATAPI", L"ATA", L"1394", L"SSA", L"FC", L"USB", L"RAID", L"ISCSI", L"SAS", L"SATA", L"SD", L"MMC", L"VIRTUAL", L"VHD", L"MAX", L"NVME"};
    WCHAR *layout[] = { L"MBR", L"GPT", L"RAW" };
    char *MBRTypes[] = { "Unused", "FAT12", "XENIX root", "XENIX /usr", "FAT16 < 32 MiB", "Extended", "FAT16", "IFS (HPFS/NTFS)", "AIX boot, OS/2, Commodore DOS", "AIX data, Coherent, QNX", "Coherent swap, OPUS, OS/2 Boot Manager", "FAT32", "FAT32 (LBA)", "Unknown", "FAT16 (LBA)", "Extended (LBA)", "OPUS", "Hidden FAT12", "Compaq diagnostics, recovery partition", "Unknown", "Hidden FAT16 < 32 MiB, AST-DOS", "Unknown", "Hidden FAT16", "Hidden IFS (HPFS/NTFS)", "AST-Windows swap", "Willowtech Photon coS", "Unknown", "Hidden FAT32", "Hidden FAT32 (LBA)", "Unknown", "Hidden FAT16 (LBA)", "Unknown", "Willowsoft Overture File System", "Oxygen FSo2", "Oxygen Extended ", "SpeedStor reserved", "NEC-DOS", "Unknown", "SpeedStor reserved", "Hidden NTFS", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "SpeedStor reserved", "Unknown", "SpeedStor reserved", "SpeedStor reserved", "Unknown", "SpeedStor reserved", "Unknown", "Theos", "Plan 9", "Unknown", "Unknown", "Partition Magic", "Hidden NetWare", "Unknown", "Unknown", "VENIX 80286", "PReP Boot", "Secure File System", "PTS-DOS", "Unknown", "Priam, EUMEL/Elan", "EUMEL/Elan", "EUMEL/Elan", "EUMEL/Elan", "Unknown", "ALFS/THIN lightweight filesystem for DOS", "Unknown", "Unknown", "QNX 4", "QNX 4", "QNX 4, Oberon", "Ontrack DM, R/O, FAT", "Ontrack DM, R/W, FAT", "CP/M, Microport UNIX", "Ontrack DM 6", "Ontrack DM 6", "EZ-Drive", "Golden Bow VFeature", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Priam EDISK", "Unknown", "Unknown", "Unknown", "Unknown", "SpeedStor", "Unknown", "GNU Hurd, System V, 386/ix", "NetWare 286", "NetWare", "NetWare 386", "NetWare", "NetWare", "NetWare NSS", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "DiskSecure Multi-Boot", "Unknown", "UNIX 7th Edition", "Unknown", "Unknown", "IBM PC/IX", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Old MINIX", "MINIX, Old Linux", "Linux swap, Solaris", "Linux", "Hidden by OS/2, APM hibernation", "Linux extended", "NT Stripe Set", "NT Stripe Set", "Linux Plaintext", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Linux LVM", "Unknown", "Unknown", "Unknown", "Unknown", "Amoeba, Hidden Linux", "Amoeba bad blocks", "Unknown", "Unknown", "Unknown", "Unknown", "Mylex EISA SCSI", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "BSD/OS", "Hibernation", "HP Volume Expansion", "Unknown", "HP Volume Expansion", "HP Volume Expansion", "FreeBSD", "OpenBSD", "NeXTStep", "Apple UFS", "NetBSD", "Olivetti DOS FAT12", "Apple Boot", "Unknown", "Unknown", "Unknown", "Apple HFS", "BootStar", "HP Volume Expansion", "Unknown", "HP Volume Expansion", "HP Volume Expansion", "Unknown", "HP Volume Expansion", "BSDi", "BSDi swap", "Unknown", "Unknown", "PTS BootWizard", "Unknown", "Unknown", "Solaris boot", "Solaris", "Novell DOS, DR-DOS secured", "DR-DOS secured FAT12", "DR-DOS reserved", "DR-DOS reserved", "DR-DOS secured FAT16 < 32 MiB", "Unknown", "DR-DOS secured FAT16", "Syrinx", "DR-DOS reserved", "DR-DOS reserved", "DR-DOS reserved", "DR-DOS secured FAT32", "DR-DOS secured FAT32 (LBA)", "DR-DOS reserved", "DR-DOS secured FAT16 (LBA)", "DR-DOS secured extended (LBA)", "Multiuser DOS secured FAT12", "Multiuser DOS secured FAT12", "Unknown", "Unknown", "Multiuser DOS secured FAT16 < 32 MiB", "Multiuser DOS secured extended", "Multiuser DOS secured FAT16", "Unknown", "CP/M", "Unknown", "Filesystem-less data", "CP/M, CCP/M, CTOS", "Unknown", "Unknown", "Dell partition", "BootIt EMBRM", "Unknown", "SpeedStor", "DOS read/only", "SpeedStor", "SpeedStor", "Tandy DOS", "SpeedStor", "Unknown", "Unknown", "Unknown", "Unknown", "BeOS", "Unknown", "Spryt*x", "Guid Partition Table", "EFI system partition", "Linux boot", "SpeedStor", "DOS 3.3 secondary, Unisys DOS", "SpeedStor", "SpeedStor", "Prologue", "SpeedStor", "Unknown", "Unknown", "Unknown", "Unknown", "VMWare VMFS", "VMWare VMKCORE", "Linux RAID, FreeDOS", "SpeedStor, LANStep, PS/2 IML", "Xenix bad block" };
    OBJECT_ATTRIBUTES attr={0};
    UNICODE_STRING diskname={0};
    WCHAR diskname_s[1024]={0};
    WCHAR guid_s[1024]={0};
    IO_STATUS_BLOCK iosb;
    NTSTATUS ret;
    int i,n;

    _snwprintf_s(diskname_s, sizeof(diskname_s) / sizeof(WCHAR), sizeof(diskname_s), L"\\??\\%s", name);
    RtlInitUnicodeString(&diskname, diskname_s);
    InitializeObjectAttributes(&attr, &diskname, 0, NULL, NULL);
    wprintf(L"%s:\n", name);

    ret=NtOpenFile(&hDisk, GENERIC_READ|SYNCHRONIZE, &attr, &iosb, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);
    if(ret!=0) {
        wprintf(L"Error: Unable to open %s, NTSTATUS=0x%08X\n\n", diskname.Buffer, ret);
        return;
    }

    // Device Property
    ret=NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_STORAGE_QUERY_PROPERTY, &desc_q, sizeof(desc_q), &desc_h, sizeof(desc_h)) ;
    if(ret==0) {
        desc_d=malloc(desc_h.Size);
        ZeroMemory(desc_d, desc_h.Size);

        ret=NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_STORAGE_QUERY_PROPERTY, &desc_q, sizeof(desc_q), desc_d, desc_h.Size);
        if(ret==0)
            if(desc_d->Version == sizeof(STORAGE_DEVICE_DESCRIPTOR)) 
                wprintf(L"  Vendor    : %S\n"
                        L"  Product   : %S\n"
                        L"  Serial    : %S\n"
                        L"  BusType   : %s\n"
                        L"  Removable : %s\n",
                        (desc_d->VendorIdOffset)     ? (char*)desc_d+desc_d->VendorIdOffset : "(n/a)", 
                        (desc_d->ProductIdOffset)    ? (char*)desc_d+desc_d->ProductIdOffset : "(n/a)",
                        (desc_d->SerialNumberOffset) ? (char*)desc_d+desc_d->SerialNumberOffset : "(n/a)",
                        (desc_d->BusType<=17)        ? bus[desc_d->BusType] : bus[0],
                        (desc_d->RemovableMedia<=1)  ? ft[desc_d->RemovableMedia] : L"(n/a)"
                );
    }

    // Status
    ret=NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_DISK_GET_DISK_ATTRIBUTES, NULL, 0, &DiskAttributes, sizeof(DiskAttributes));
    if(ret==0) 
        wprintf(L"  Status    : %s \n", (DiskAttributes.Attributes) ? L"Offline" : L"Online");
    else
        wprintf(L"  Status    : (n/a)\n");

    // Media
    ret=NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_STORAGE_CHECK_VERIFY2, NULL, 0, NULL, 0);
    wprintf(L"  Media     : %s\n", (ret==0) ? L"Present" : L"Not-present");

    // Readonly
    ret=NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_DISK_IS_WRITABLE, NULL, 0, NULL, 0);
    wprintf(L"  Readonly  : %s \n", (ret<0) ? L"True" : L"False");


    // Size
    ret=NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &DiskLengthInfo, sizeof(DiskLengthInfo));
    if(ret==0)
        wprintf(L"  Size      : %.1f GB \n", (float)DiskLengthInfo.Length.QuadPart / 1024.0 / 1024.0 / 1024.0);
    else
        wprintf(L"  Size      : (n/a)\n");


    // Trim
    ret=NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_STORAGE_QUERY_PROPERTY, &trim_q, sizeof(trim_q), &trim_d, sizeof(trim_d));
    if(ret==0)
        wprintf(L"  Trim      : %s\n", (trim_d.Version == sizeof(DEVICE_TRIM_DESCRIPTOR) && trim_d.TrimEnabled == 1) ? L"Supported" : L"Not Supported");
    else
        wprintf(L"  Trim      : (n/a)\n");

    // SCSI Address
    ret=NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_SCSI_GET_ADDRESS, NULL, 0, &DiskAddress, sizeof(DiskAddress));
    if(ret==0)
        wprintf(L"  HBTL      : %d:%d:%d:%d \n", DiskAddress.PortNumber, DiskAddress.PathId, DiskAddress.TargetId, DiskAddress.Lun);
    else
        wprintf(L"  HBTL      : (n/a)\n");


    // Layout
    ret=NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, &DiskLayout, sizeof(DiskLayout));
    if(ret==0) {
        if(DiskLayout->PartitionStyle == 1) {
                StringFromGUID2(&DiskLayout->Gpt.DiskId, guid_s, sizeof(guid_s));
                wprintf(L"  DiskID:   : %s \n", guid_s);
        }
        else if (DiskLayout->PartitionStyle == 0) {
                wprintf(L"  DiskID:   : %X \n", DiskLayout->Mbr.Signature);
        }
        wprintf(L"  Layout    : %s\n", layout[DiskLayout->PartitionStyle]);
        
        for(n=0;n<DiskLayout->PartitionCount;n++)  {
            if(DiskLayout->PartitionEntry[n].PartitionNumber) {
                wprintf(L"    Partition %d:\n      Style : %s\n", 
                    DiskLayout->PartitionEntry[n].PartitionNumber,
                    layout[DiskLayout->PartitionEntry[n].PartitionStyle]
                );
                if(DiskLayout->PartitionEntry[n].PartitionStyle==PARTITION_STYLE_MBR) {
                    wprintf(L"      Type  : 0x%0X [%S] %s %s\n", 
                        DiskLayout->PartitionEntry[n].Mbr.PartitionType, 
                        MBRTypes[DiskLayout->PartitionEntry[n].Mbr.PartitionType],
                        (DiskLayout->PartitionEntry[n].Mbr.BootIndicator) ? L"[Active]" : L"",
                        (DiskLayout->PartitionEntry[n].Mbr.RecognizedPartition) ? L"[Recognized]" : L""
                    );
                }
                else if(DiskLayout->PartitionEntry[n].PartitionStyle==PARTITION_STYLE_GPT) {
                    StringFromGUID2(&DiskLayout->PartitionEntry[n].Gpt.PartitionType, guid_s, sizeof(guid_s));
                    wprintf(L"      Type  : %s ", guid_s);

                    if(IsEqualGUID(&DiskLayout->PartitionEntry[n].Gpt.PartitionType, &PARTITION_ENTRY_UNUSED_GUID)) wprintf(L"[Unused]");
                    else if(IsEqualGUID(&DiskLayout->PartitionEntry[n].Gpt.PartitionType, &PARTITION_SYSTEM_GUID)) wprintf(L"[EFI System]");
                    else if(IsEqualGUID(&DiskLayout->PartitionEntry[n].Gpt.PartitionType, &PARTITION_MSFT_RESERVED_GUID)) wprintf(L"[MSFT Reserved]");
                    else if(IsEqualGUID(&DiskLayout->PartitionEntry[n].Gpt.PartitionType, &PARTITION_BASIC_DATA_GUID)) wprintf(L"[Basic Data]");
                    else if(IsEqualGUID(&DiskLayout->PartitionEntry[n].Gpt.PartitionType, &PARTITION_LDM_METADATA_GUID)) wprintf(L"[LDM Metadata]");
                    else if(IsEqualGUID(&DiskLayout->PartitionEntry[n].Gpt.PartitionType, &PARTITION_LDM_DATA_GUID)) wprintf(L"[LDM Data]");
                    else if(IsEqualGUID(&DiskLayout->PartitionEntry[n].Gpt.PartitionType, &PARTITION_MSFT_RECOVERY_GUID)) wprintf(L"[MSFT Recovery]");
                    else if(IsEqualGUID(&DiskLayout->PartitionEntry[n].Gpt.PartitionType, &PARTITION_CLUSTER_GUID)) wprintf(L"[Cluster Metadata]");
                    else if(IsEqualGUID(&DiskLayout->PartitionEntry[n].Gpt.PartitionType, &PARTITION_MSFT_SNAPSHOT_GUID)) wprintf(L"[Shadow Copy]");
                    else if(IsEqualGUID(&DiskLayout->PartitionEntry[n].Gpt.PartitionType, &PARTITION_SPACES_GUID)) wprintf(L"[Storage Spaces]");
                    else wprintf(L"[Unrecognized]");
                    wprintf(L"\n");

                    StringFromGUID2(&DiskLayout->PartitionEntry[n].Gpt.PartitionId, guid_s, sizeof(guid_s));
                    wprintf(L"      ID    : %s\n      Name  : %s\n      Attrib: 0x%I64X ", 
                        guid_s, 
                        DiskLayout->PartitionEntry[n].Gpt.Name,
                        DiskLayout->PartitionEntry[n].Gpt.Attributes
                    );

                    if((DiskLayout->PartitionEntry[n].Gpt.Attributes & GPT_ATTRIBUTE_PLATFORM_REQUIRED) == GPT_ATTRIBUTE_PLATFORM_REQUIRED) wprintf(L"[Required] ");
                    if((DiskLayout->PartitionEntry[n].Gpt.Attributes & GPT_BASIC_DATA_ATTRIBUTE_NO_DRIVE_LETTER) == GPT_BASIC_DATA_ATTRIBUTE_NO_DRIVE_LETTER) wprintf(L"[No_Letter] ");
                    if((DiskLayout->PartitionEntry[n].Gpt.Attributes & GPT_BASIC_DATA_ATTRIBUTE_HIDDEN) == GPT_BASIC_DATA_ATTRIBUTE_HIDDEN) wprintf(L"[Hidden] ");
                    if((DiskLayout->PartitionEntry[n].Gpt.Attributes & GPT_BASIC_DATA_ATTRIBUTE_SHADOW_COPY) == GPT_BASIC_DATA_ATTRIBUTE_SHADOW_COPY) wprintf(L"[Shadow_Copy] ");
                    if((DiskLayout->PartitionEntry[n].Gpt.Attributes & GPT_BASIC_DATA_ATTRIBUTE_READ_ONLY) == GPT_BASIC_DATA_ATTRIBUTE_READ_ONLY) wprintf(L"[Readonly] ");
                    wprintf(L"\n");

                }
                wprintf(L"      Offset: %llu (%.1f GB)\n      Length: %llu (%.1f GB)\n", 
                    DiskLayout->PartitionEntry[n].StartingOffset.QuadPart,
                    (float)DiskLayout->PartitionEntry[n].StartingOffset.QuadPart/1024/1024/1024,
                    DiskLayout->PartitionEntry[n].PartitionLength.QuadPart,
                    (float)DiskLayout->PartitionEntry[n].PartitionLength.QuadPart/1024/1024/1024
                );
            }
        };

    }
    else
        wprintf(L"  Layout    : (n/a)\n");


    wprintf(L"\n");

    NtClose(hDisk);
}



int wmain(int argc, WCHAR **argv) {

    wprintf(L"ListDisk v3.3, Copyright (c) 2018 by Antoni Sawicki\n\n");

    ListDisk();

    return 0;
}