    
#include <iostream>
#include <windows.h>
#include <string>
#include  <iomanip>

using namespace std;

bool ReadClusters(HANDLE disk, BYTE buffer[], int NumberCluster, int CountClusters,int SizeCluster)
{
    bool result = TRUE;
    LARGE_INTEGER offset;
    offset.QuadPart = (__int64)NumberCluster * (__int64)SizeCluster;  
    DWORD BytesRead;
    auto pointer = SetFilePointer(disk, offset.LowPart, &offset.HighPart, FILE_BEGIN);
    auto readResult = ReadFile(disk, buffer, CountClusters * SizeCluster, &BytesRead, NULL);
    if (!readResult || (CountClusters * SizeCluster) != BytesRead)
    {
        cout << "Ошибка чтения кластеров!" << endl;
        return -10;
    }
    else
        return result;
}



int main()
{
	setlocale(LC_ALL, "ru");
    cout << "Данная программа покажет информацию об установленной файловой системе" <<endl << "===================================================" << endl;
    cout << "Доступные тома:" << endl;
    auto mask = GetLogicalDrives();
    int o = 0;
    for (int x = 0; x < 26; x++) // прогоняем по битам
    {
        auto n = ((mask >> x) & 1); // определяем значение текущего бита
        if (n) // если единица - диск с номером x есть
        {
            o++;
            cout << o << ") " << (char)(65 + x) << endl;; // получаем литеру диска
        }
    }
    
    cout << endl << "Укажите Букву тома (A-Z):" << endl;
    wstring tom;
    wcin >> tom;
    wstring filename = L"\\\\.\\" + tom + L":";
	auto OpenDisk = CreateFileW(filename.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (OpenDisk == INVALID_HANDLE_VALUE)
	{
		cout << "Не удалось открыть диск для чтения" << endl;
		return -1;
	}
	BYTE buffer[0x200];
	DWORD nBytesRead = 0x200, bytesRead = 0;
	auto ReadDisk = ReadFile(OpenDisk, buffer, nBytesRead, &bytesRead, NULL);
	if (!ReadDisk || bytesRead != nBytesRead)
	{
		cout << "Ошибка чтения данных с диска!" << endl;
		cout << GetLastError() << endl;
		return -2;
	}
#pragma pack(push,1)
    typedef struct _BOOT_NTFS
    {
        BYTE    jump[3];
        BYTE    name[8];
        UINT16  sec_size;
        BYTE    secs_cluster;
        BYTE    reserved_0[7];
        BYTE    media_desc;
        UINT16  reserved_2;
        UINT16  secs_track;
        UINT16  num_heads;
        BYTE    reserved_3[8];
        UINT16  reserved_4;
        UINT16  reserved_5;
        UINT64  num_secs;
        UINT64  mft_clus;
        UINT64  mft2_clus;
        UINT32  mft_rec_size;
        UINT32  buf_size;
        UINT64  volume_id;
    } BOOT_NTFS;
#pragma pack(pop)
    BOOT_NTFS *my_NTFS;
    my_NTFS = (BOOT_NTFS*)buffer;
    int size_cluster = my_NTFS->sec_size * my_NTFS->secs_cluster;
    cout << "Информация о файловой системе диска:" << endl;
    cout << "Файловая система:\t" << my_NTFS->name << endl;
    cout << "Количество байт в секторе:\t" << my_NTFS->sec_size << endl;
    cout << "Количество секторов в кластере:\t" << int(my_NTFS->secs_cluster) << endl;
    cout << "Количество байт в кластере:\t" << size_cluster << endl;
    cout << "Секторов в разделе:\t" << my_NTFS->num_secs << endl;
    cout << "Номер кластера, в котором начинается таблица MFT:\t" << my_NTFS->mft_clus << endl;
    cout << "Номер кластера, в котором начинается копия таблицы MFT:\t" << my_NTFS->mft2_clus << endl;
    cout << "Размер таблицы MFT в кластерах:\t" << my_NTFS->mft_rec_size << endl;
    cout << "Серийный номер диска:\t" << my_NTFS->volume_id << endl;

    cout << "==========================================================" << endl;
    cout << "Считать кластеры?\t(1 - да, 0 - выход)" << endl;
    short mode;
    cin >> mode;
    switch (mode)
    {
    case 0:
    {
        CloseHandle(OpenDisk);
        system("pause");
        return 0;
    }
    case 1:
    {
        int FirstCluster,CountClusters;
        cout << "Введите номер кластера, с которого нужно начать чтение: ";
        cin >> FirstCluster;
        cout << endl << "Введите количество кластеров, которые необходимо прочитать: ";
        cin >> CountClusters;
        cout << endl;
        __int64 p = (__int64)FirstCluster * (__int64)size_cluster;
        BYTE* Data = new BYTE[CountClusters * size_cluster];
        auto readclusters = ReadClusters(OpenDisk, Data, FirstCluster, CountClusters, size_cluster);
        for (int i = 0; i < CountClusters * size_cluster; i++)
        {
            cout << std::hex <<setw(2)<<setfill('0')<<uppercase<< int(Data[i]) << ' ';
                if ((i + 1) % 16 == 0)
                    cout << endl;
        }
        delete[] Data;
        CloseHandle(OpenDisk);
        system("pause");
        return 0;
    }
    default:
    {
        cout << "Некорректный ввод, возможные варианты: 1 или 0" << endl;
        CloseHandle(OpenDisk);
        system("pause");
        return 0;
    
    }
    }	
}

