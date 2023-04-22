#include <iostream>
#include <Windows.h>
#include <string>

std::string readRegistryValue(HKEY hKey, std::string keyPath, std::string valueName) {
    HKEY hRegistryKey;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    std::string valueData;

    if (RegOpenKeyExA(hKey, keyPath.c_str(), 0, KEY_READ, &hRegistryKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hRegistryKey, valueName.c_str(), nullptr, &dwType, nullptr, &dwSize) == ERROR_SUCCESS) {
            BYTE* buffer = new BYTE[dwSize];

            if (RegQueryValueExA(hRegistryKey, valueName.c_str(), nullptr, &dwType, buffer, &dwSize) == ERROR_SUCCESS) {
                if (dwType == REG_SZ) {
                    valueData = reinterpret_cast<char*>(buffer);
                }
            }

            delete[] buffer;
        }

        RegCloseKey(hRegistryKey);
    }

    return valueData;
}

void writeRegistryValue(HKEY hKey, std::string keyPath, std::string valueName, std::string valueData) {
	HKEY hRegistryKey;
	DWORD dwDisposition = 0;

	if (RegCreateKeyExA(hKey, keyPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hRegistryKey, &dwDisposition) == ERROR_SUCCESS) {
		RegSetValueExA(hRegistryKey, valueName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(valueData.c_str()), valueData.length() + 1);
		RegCloseKey(hRegistryKey);
	}
}

DWORD getVolumeSerialNumber() {
    DWORD serialNumber = 0;
    GetVolumeInformation(L"C:\\", NULL, 0, &serialNumber, NULL, NULL, NULL, 0);
    return serialNumber;
}

int parseAppVer(const std::string& appVer) {
    std::string appVerParsed = appVer.substr(14);
    appVerParsed.erase(std::remove(appVerParsed.begin(), appVerParsed.end(), '.'), appVerParsed.end());
    return std::stoi(appVerParsed);
}

std::string genRegKeyName(DWORD serialNumber, int appVer) {
    std::string regKey = "SN";
    int magicNum = appVer ^ ((~serialNumber >> 1) + 736) ^ 0x590D4;
	regKey += std::to_string(magicNum);
	return regKey;
}

std::string genLicenseKey() {
	int v1 = rand() % 255;
	int v2 = rand() % 255;
	int v3 = rand();
	char buffer[17];
	sprintf_s(buffer, sizeof(buffer), "%02X%02X%02X7C%02X%02X%02X%02X", v1, v2 ^ 0x7C, (unsigned __int8)~(BYTE)v1, v2, v3 % 255, (unsigned __int8)(v3 % 255) ^ 7, v1 ^ (unsigned __int8)~(v3 % 255));
	return std::string(buffer);
}

int main() {
	srand(time(NULL));
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	
    int appVer = parseAppVer(readRegistryValue(HKEY_CURRENT_USER, "Software\\MadeForNet\\HTTPDebuggerPro", "AppVer"));
	std::cout << "HTTPDebugger Version:   ";
	SetConsoleTextAttribute(hConsole, 14);
	std::cout << appVer << std::endl;

	SetConsoleTextAttribute(hConsole, 7);
	DWORD serialNumber = getVolumeSerialNumber();
	std::cout << "Volume Serial Number:   ";
	SetConsoleTextAttribute(hConsole, 14);
	std::cout << serialNumber << std::endl;
	
	SetConsoleTextAttribute(hConsole, 7);
	std::string regKey = genRegKeyName(serialNumber, appVer);
	std::cout << "Generated Registry Key: ";
	SetConsoleTextAttribute(hConsole, 14);
	std::cout << regKey << std::endl;
	
	SetConsoleTextAttribute(hConsole, 7);
	std::string licenseKey = genLicenseKey();
	std::cout << "Generated License Key:  ";
	SetConsoleTextAttribute(hConsole, 14);
	std::cout << licenseKey << std::endl;

	std::cout << std::endl;
	std::cout << "Writing to registry..." << std::endl;
	writeRegistryValue(HKEY_CURRENT_USER, "Software\\MadeForNet\\HTTPDebuggerPro", regKey, licenseKey);
	SetConsoleTextAttribute(hConsole, 10);
	std::cout << "Done!" << std::endl;

	std::cin.get();
}