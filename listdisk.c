//
// List and Query Physical Disk Properties on Windows NT based systes
// Copyright (c) 2016 by Antoni Sawicki
//
// v2.3, as@tenoware.com
//

#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <winternl.h>
#include <Ntddscsi.h>

#define DIRECTORY_QUERY                 (0x0001)
#define DIRECTORY_TRAVERSE              (0x0002)

typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;

#pragma comment(lib, "ntdll.lib")

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
    SCSI_ADDRESS DiskAddress;
    STORAGE_PROPERTY_QUERY desc_q = { StorageDeviceProperty,  PropertyStandardQuery };
    STORAGE_DESCRIPTOR_HEADER desc_h = { 0 };
    PSTORAGE_DEVICE_DESCRIPTOR desc_d;
    STORAGE_PROPERTY_QUERY trim_q = { StorageDeviceTrimProperty,  PropertyStandardQuery };
    DEVICE_TRIM_DESCRIPTOR trim_d = { 0 };
    WCHAR *ft[] = { L"False", L"True" };
    WCHAR *bus[] = { L"UNKNOWN", L"SCSI", L"ATAPI", L"ATA", L"1394", L"SSA", L"FC", L"USB", L"RAID", L"ISCSI", L"SAS", L"SATA", L"SD", L"MMC", L"VIRTUAL", L"VHD", L"MAX", L"NVME",L"UNKNOWN",L"UNKNOWN",L"UNKNOWN",L"UNKNOWN",L"UNKNOWN",L"UNKNOWN",L"UNKNOWN",L"UNKNOWN",L"UNKNOWN" };
    OBJECT_ATTRIBUTES attr={0};
    UNICODE_STRING diskname={0};
    WCHAR diskname_s[1024]={0};
    IO_STATUS_BLOCK iosb;
    NTSTATUS ret;

    _snwprintf_s(diskname_s, sizeof(diskname_s) / sizeof(WCHAR), sizeof(diskname_s), L"\\??\\%s", name);
    RtlInitUnicodeString(&diskname, diskname_s);
    InitializeObjectAttributes(&attr, &diskname, 0, NULL, NULL);
    wprintf(L"%s:\n", name);
    //wprintf(L"%s:\n", diskname.Buffer);

    ret=NtOpenFile(&hDisk, GENERIC_READ|SYNCHRONIZE, &attr, &iosb, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);
    if(ret!=0)
        return;

    // Device Property
    ret=NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_STORAGE_QUERY_PROPERTY, &desc_q, sizeof(desc_q), &desc_h, sizeof(desc_h)) ;
    if(ret==0) {
        desc_d=malloc(desc_h.Size);
        ZeroMemory(desc_d, desc_h.Size);

        ret=NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_STORAGE_QUERY_PROPERTY, &desc_q, sizeof(desc_q), desc_d, desc_h.Size);
        if(ret==0)
            if(desc_d->Version == sizeof(STORAGE_DEVICE_DESCRIPTOR)) 
                wprintf(L"  Vendor:    %S\n  Product:   %S\n  Serial:    %S\n  Removable: %s\n  BusType:   %s\n", 
                        (desc_d+desc_d->VendorIdOffset)     ? (char*)desc_d+desc_d->VendorIdOffset : "(n/a)", 
                        (desc_d+desc_d->ProductIdOffset)    ? (char*)desc_d+desc_d->ProductIdOffset : "(n/a)",
                        (desc_d+desc_d->SerialNumberOffset) ? (char*)desc_d+desc_d->SerialNumberOffset : "(n/a)",
                        ft[desc_d->RemovableMedia],
                        bus[desc_d->BusType]
                );
    }

    // Size
    ret=NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &DiskLengthInfo, sizeof(DiskLengthInfo));
    if(ret==0)
        wprintf(L"  Size:      %.1f GB \n", (float)DiskLengthInfo.Length.QuadPart / 1024.0 / 1024.0 / 1024.0);


    // Trim
    ret=NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_STORAGE_QUERY_PROPERTY, &trim_q, sizeof(trim_q), &trim_d, sizeof(trim_d));
    if(ret==0 && trim_d.Version == sizeof(DEVICE_TRIM_DESCRIPTOR) && trim_d.TrimEnabled == 1)
        wprintf(L"  Trim:      Supported\n");
    else
        wprintf(L"  Trim:      Not Supported\n");

    // SCSI Address
    ret=NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_SCSI_GET_ADDRESS, NULL, 0, &DiskAddress, sizeof(DiskAddress));
    if(ret==0)
        wprintf(L"  HBTL:      %d:%d:%d:%d \n", DiskAddress.PortNumber, DiskAddress.PathId, DiskAddress.TargetId, DiskAddress.Lun);


    wprintf(L"\n");

    NtClose(hDisk);
}



int wmain(int argc, WCHAR **argv) {

    wprintf(L"ListDisk v2.3, Copyright (c) 2017 by Antoni Sawicki\n\n");

    ListDisk();

    return 0;
}