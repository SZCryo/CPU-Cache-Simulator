// Lalafarian, Edgar        CS230 Section 10989 5/4/2026
// Fourth Laboratory Assignment - Cache Simulation

#include <array>
#include <cctype>
#include <iostream>

using namespace std;

const int LINE_SIZE = 16;
const int CACHE_SIZE = 2048;
const int WAYS = 2;
const int MEMORY_SIZE = 32768;
const int INT_SIZE = 4;
const int WORDS_PER_LINE = LINE_SIZE / INT_SIZE;
const int SETS = (CACHE_SIZE / LINE_SIZE) / WAYS;
const int MEMORY_WORDS = MEMORY_SIZE / INT_SIZE;
const int ADDRESS_BITS = 16;

struct CacheLine {
    array<int, WORDS_PER_LINE> values{};
    int tag = -1;
    int lineStart = 0;
    bool valid = false;
    bool dirty = false;
    unsigned long long lastUsed = 0;
};

array<int, MEMORY_WORDS> mainMemory{};
array<array<CacheLine, WAYS>, SETS> cacheSets{};
unsigned long long timeCounter = 0;

int bitsNeeded(int value) {
    int bits = 0;
    int power = 1;

    while (power < value) {
        power *= 2;
        bits++;
    }

    return bits;
}

const int OFFSET_BITS = bitsNeeded(LINE_SIZE);
const int SET_BITS = bitsNeeded(SETS);
const int TAG_BITS = ADDRESS_BITS - OFFSET_BITS - SET_BITS;

int lowerMultiple(int value, int multiple) {
    return value - value % multiple;
}

bool validAddress(int &address) {
    int original = address;

    if (address < 0) {
        cout << "Input address " << original << " invalid" << endl;
        return false;
    }

    if (address % INT_SIZE != 0) {
        address = lowerMultiple(address, INT_SIZE);
        cout << "Setting address to next lower multiple of 4" << endl;
    }

    if (address + INT_SIZE > MEMORY_SIZE) {
        cout << "Input address " << original << " invalid" << endl;
        return false;
    }

    return true;
}

int setNumber(int address) {
    return (address >> OFFSET_BITS) & (SETS - 1);
}

int tagValue(int address) {
    return (address >> (OFFSET_BITS + SET_BITS)) & ((1 << TAG_BITS) - 1);
}

int baseAddress(int address) {
    return lowerMultiple(address, LINE_SIZE);
}

int wordOffset(int address) {
    return (address % LINE_SIZE) / INT_SIZE;
}

void printWords(const array<int, WORDS_PER_LINE> &words) {
    for (int index = 0; index < WORDS_PER_LINE; index++) {
        cout << words[index];
        if (index < WORDS_PER_LINE - 1) {
            cout << ' ';
        }
    }
}

void writeBack(CacheLine &line) {
    if (!line.valid || !line.dirty) {
        return;
    }

    int memoryStart = line.lineStart / INT_SIZE;
    for (int index = 0; index < WORDS_PER_LINE; index++) {
        mainMemory[memoryStart + index] = line.values[index];
    }
    line.dirty = false;
}

CacheLine *findLine(int address) {
    int set = setNumber(address);
    int tag = tagValue(address);

    for (int way = 0; way < WAYS; way++) {
        if (cacheSets[set][way].valid && cacheSets[set][way].tag == tag) {
            return &cacheSets[set][way];
        }
    }

    return nullptr;
}

CacheLine &lineToUse(int set) {
    if (!cacheSets[set][0].valid) {
        return cacheSets[set][0];
    }
    if (!cacheSets[set][1].valid) {
        return cacheSets[set][1];
    }
    if (cacheSets[set][0].lastUsed <= cacheSets[set][1].lastUsed) {
        return cacheSets[set][0];
    }
    return cacheSets[set][1];
}

CacheLine &loadLine(int address) {
    int set = setNumber(address);
    int firstAddress = baseAddress(address);
    CacheLine &line = lineToUse(set);

    writeBack(line);

    int memoryStart = firstAddress / INT_SIZE;
    for (int index = 0; index < WORDS_PER_LINE; index++) {
        line.values[index] = mainMemory[memoryStart + index];
    }

    line.tag = tagValue(address);
    line.lineStart = firstAddress;
    line.valid = true;
    line.dirty = false;
    line.lastUsed = ++timeCounter;
    return line;
}

CacheLine &getLine(int address) {
    CacheLine *line = findLine(address);
    if (line == nullptr) {
        return loadLine(address);
    }

    line->lastUsed = ++timeCounter;
    return *line;
}

void readMemory(int address) {
    getLine(address);
}

void writeMemory(int address, int value) {
    CacheLine &line = getLine(address);
    line.values[wordOffset(address)] = value;
    line.dirty = true;
    line.lastUsed = ++timeCounter;
}

void displayAddress(int address) {
    int set = setNumber(address);
    int offset = wordOffset(address);
    int firstCacheValue = cacheSets[set][0].valid ? cacheSets[set][0].values[offset] : -1;
    int secondCacheValue = cacheSets[set][1].valid ? cacheSets[set][1].values[offset] : -1;

    cout << "Address: " << address
         << " memory:" << mainMemory[address / INT_SIZE]
         << " cache:" << firstCacheValue << ' ' << secondCacheValue << ' '
         << cacheSets[set][0].valid << ' ' << cacheSets[set][0].dirty << ' '
         << cacheSets[set][1].valid << ' ' << cacheSets[set][1].dirty << endl;

    int firstAddress = baseAddress(address);
    cout << "Address: " << firstAddress << " memory:";
    for (int index = 0; index < WORDS_PER_LINE; index++) {
        cout << mainMemory[firstAddress / INT_SIZE + index];
        if (index < WORDS_PER_LINE - 1) {
            cout << ' ';
        }
    }
    cout << endl;

    for (int way = 0; way < WAYS; way++) {
        cout << "Cache:";
        if (cacheSets[set][way].valid) {
            printWords(cacheSets[set][way].values);
        } else {
            cout << -1;
        }
        cout << ' ' << cacheSets[set][way].valid << ' ' << cacheSets[set][way].dirty << endl;
    }
}

bool hasData(const array<int, WORDS_PER_LINE> &line) {
    for (int value : line) {
        if (value != 0) {
            return true;
        }
    }
    return false;
}

void dumpCache() {
    for (int set = 0; set < SETS; set++) {
        for (int way = 0; way < WAYS; way++) {
            CacheLine &line = cacheSets[set][way];
            if (line.valid && hasData(line.values)) {
                cout << "Address: " << line.lineStart << " Cache:";
                printWords(line.values);
                cout << ' ' << line.tag << ' ' << line.valid << ' ' << line.dirty << endl;
            }
        }
    }
}

void dumpMemory() {
    for (int firstAddress = 0; firstAddress < MEMORY_SIZE; firstAddress += LINE_SIZE) {
        array<int, WORDS_PER_LINE> line{};
        for (int index = 0; index < WORDS_PER_LINE; index++) {
            line[index] = mainMemory[firstAddress / INT_SIZE + index];
        }

        if (hasData(line)) {
            cout << "Address: " << firstAddress << " memory:";
            printWords(line);
            cout << endl;
        }
    }
}

void evictAddress(int address) {
    CacheLine *line = findLine(address);
    if (line == nullptr) {
        cout << "Not in cache." << endl;
        return;
    }

    writeBack(*line);
    *line = CacheLine{};
}

int main() {
    cout << "CS230 Lab Assignment 3 - Lalafarian, Edgar" << endl;

    char command;
    while (true) {
        cout << "Enter a command, A or B:" << endl;
        if (!(cin >> command)) {
            break;
        }

        command = toupper(command);

        if (command == 'A') {
            int address;
            int value = 0;
            char request;

            cin >> address >> request;
            request = toupper(request);

            if (request != 'R' && request != 'W') {
                cout << "Request type must be R or W" << endl;
                continue;
            }

            if (request == 'W') {
                cin >> value;
            }
            if (!validAddress(address)) {
                continue;
            }

            if (request == 'R') {
                readMemory(address);
            } else {
                writeMemory(address, value);
            }
        } else if (command == 'B') {
            int address;
            cin >> address;

            if (address == -1) {
                cout << "Done." << endl;
                break;
            }
            if (validAddress(address)) {
                displayAddress(address);
            }
        } else if (command == 'C') {
            dumpCache();
        } else if (command == 'D') {
            dumpMemory();
        } else if (command == 'E') {
            int address;
            cin >> address;

            if (validAddress(address)) {
                evictAddress(address);
            }
        } else {
            cout << "Command must be A, a, B, or b" << endl;
        }
    }

    return 0;
}
