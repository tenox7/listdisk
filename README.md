# listdisk
## List and Query Physical Disk Properties on Microsoft Windows based systes.

Displays vendor, product, bus type, serial number, disk GUIDs used by Microsoft Failover Cluster, BTL, trim support, etc. The utility will work even if VDS is hung or inoperable. 

Developer example of how to list physical disks in Windows NT systems by querying NT Namespace for PhysicalDrive objects and query physical disk for properties via IOCTL.

Example output:
```
ListDisk v3.3, Copyright (c) 2018 by Antoni Sawicki

PhysicalDrive0:
  Vendor    : (n/a)
  Product   : SAMSUNG MXAERTNLERNE-00834
  Serial    : S45245634569823
  BusType   : SATA
  Removable : False
  Status    : Online 
  Media     : Present
  Readonly  : False 
  Size      : 3577.0 GB 
  Trim      : Supported
  HBTL      : 0:1:0:0 
  DiskID:   : {3F41CEA1-B015-4334-8C79-1F4BB5F0299D} 
  Layout    : GPT
    Partition 1:
      Style : GPT
      Type  : {5808C8AA-7E8F-42E0-85D2-E1E90434CFB3} [LDM Metadata]
      ID    : {5D494D92-F0B3-11E6-A698-382C4A6D5FDC}
      Name  : LDM metadata partition
      Attrib: 0x0 
      Offset: 17408 (0.0 GB)
      Length: 1048576 (0.0 GB)
    Partition 2:
      Style : GPT
      Type  : {E3C9E316-0B5C-4DB8-817D-F92DF00215AE} [MSFT Reserved]
      ID    : {FC2D8E2D-0252-41A5-A453-9094744A6C04}
      Name  : Microsoft reserved partition
      Attrib: 0x0 
      Offset: 1065984 (0.0 GB)
      Length: 133169152 (0.1 GB)
    Partition 3:
      Style : GPT
      Type  : {AF9B60A0-1431-4F62-BC68-3311714A69AD} [LDM Data]
      ID    : {5D494D96-F0B3-11E6-A698-382C4A6D5FDC}
      Name  : LDM data partition
      Attrib: 0x0 
      Offset: 134235136 (0.1 GB)
      Length: 3840621730304 (3576.9 GB)

PhysicalDrive1:
  Vendor    : NVMe    
  Product   : Samsung SSD 950 
  Serial    : 0000_0000_0000_0000.
  BusType   : NVME
  Removable : False
  Status    : Online 
  Media     : Present
  Readonly  : False 
  Size      : 476.9 GB 
  Trim      : Supported
  HBTL      : 2:0:0:0 
  DiskID:   : {D0C45AEA-586A-4DF2-A657-6C8E3C0A487A} 
  Layout    : GPT
    Partition 1:
      Style : GPT
      Type  : {DE94BBA4-06D1-4D40-A16A-BFD50179D6AC} [MSFT Recovery]
      ID    : {C3FAD329-0ECB-4C4E-9335-DC5A35927458}
      Name  : Basic data partition
      Attrib: 0x8000000000000001 [Required] [No_Letter] 
      Offset: 1048576 (0.0 GB)
      Length: 471859200 (0.4 GB)
    Partition 2:
      Style : GPT
      Type  : {C12A7328-F81F-11D2-BA4B-00A0C93EC93B} [EFI System]
      ID    : {A17F368F-02ED-478B-A48A-AA1736D3616B}
      Name  : EFI system partition
      Attrib: 0x8000000000000000 [No_Letter] 
      Offset: 472907776 (0.4 GB)
      Length: 103809024 (0.1 GB)
    Partition 3:
      Style : GPT
      Type  : {E3C9E316-0B5C-4DB8-817D-F92DF00215AE} [MSFT Reserved]
      ID    : {D335F984-225C-489E-99E7-1F74604F2194}
      Name  : Microsoft reserved partition
      Attrib: 0x8000000000000000 [No_Letter] 
      Offset: 576716800 (0.5 GB)
      Length: 16777216 (0.0 GB)
    Partition 4:
      Style : GPT
      Type  : {EBD0A0A2-B9E5-4433-87C0-68B6B72699C7} [Basic Data]
      ID    : {593E4DB8-B1AA-49B1-84D9-040959A4DCF1}
      Name  : Basic data partition
      Attrib: 0x0 
      Offset: 593494016 (0.6 GB)
      Length: 511516344320 (476.4 GB)

```
