#include "Wad.h"

Directory* newDir(string name, bool isDir, Directory* parent) { //Tree node
    Directory* dir = new Directory;
    dir->name = name;
    dir->isDir = isDir;
    dir->parent = parent;
    return dir;
};

Wad* Wad::loadWad(const string &path) { //Load WAD
    fstream file(path, ios_base::in);
    Wad* wad = new Wad();
    wad->root = newDir("/", true, nullptr); //Initialize root directory

    for(int i = 0; i < 4; ++i) { //Read in magic
        file.read(&wad->magic[i], 1);
    }

    string magic(wad->magic);
    if(magic.find("WAD") == string::npos) { //Check if magic is valid
        return nullptr;
    }

    file.read((char*)&wad->numDesc, sizeof(int)); //Read number of descriptors

    file.read((char*)&wad->descOffset, sizeof(int)); //Read offset of first descriptor

    file.seekg(wad->descOffset, file.beg);
    for(unsigned int i = 0; i < wad->numDesc; ++i) { //Read descriptors in loop
        Descriptor desc;

        file.read((char*)&desc.elemOffset, sizeof(int));

        file.read((char*)&desc.elemLength, sizeof(int));

        char c[8];
        for(int i = 0; i < 8; ++i) {
            file.read(&c[i], 1);
            desc.name += c[i];
        }

        wad->descriptors.push_back(desc); //Add descriptor to WAD vector
    }

    int mapCount = 10;
    bool inMap = false;
    Directory* cur = wad->root;
    unsigned int e = wad->descriptors.size();
    for(unsigned int i = 0; i < wad->descriptors.size(); ++i) { //Read lumps of each descriptor and add to tree
        file.seekg(wad->descriptors[i].elemOffset, ios_base::beg);

        //Map marker check
        bool isMap =
                (wad->descriptors[i].name[0] == 'E'
                 && isdigit(wad->descriptors[i].name[1])
                 && wad->descriptors[i].name[2] == 'M'
                 && isdigit(wad->descriptors[i].name[3]));

        string curName(wad->descriptors[i].name);

        bool startNamespace = (curName.find("_START") != string::npos); //Namespace marker begin check
        bool endNamespace = (curName.find("_END") != string::npos); //end check

        if(mapCount == 0) { //Reset to parent directory at end of map marker
            mapCount = 10;
            inMap = false;
            cur = cur->parent;
        }

        if(isMap) { //Create new map dir
            cur = newDir(curName, true, cur);
            inMap = true;
            cur->parent->childDir.push_back(cur);
            continue;

        } else if(startNamespace) { //Create new sub-dir
            string name = wad->descriptors[i].name;
            name.erase(name.find("_START"), string::npos);
            cur = newDir(name, true, cur);
            cur->parent->childDir.push_back(cur);
            continue;

        } else if(endNamespace) { //End sub-dir and return to parent
            cur = cur->parent;
            continue;
        }

        if(inMap) {//read ten
            Directory* dir = newDir(wad->descriptors[i].name, false, cur);

            char c[wad->descriptors[i].elemLength];
            string s = "";
            for(unsigned int j = 0; j < wad->descriptors[i].elemLength; ++j) {
                file.read(&c[j], 1);
                s += c[j];
            }
            dir->lump = s;

            cur->childDir.push_back(dir);
            mapCount--;
        } else { //read normal
            Directory* dir = newDir(wad->descriptors[i].name, false, cur);

            char c[wad->descriptors[i].elemLength];
            string s = "";
            for(unsigned int j = 0; j < wad->descriptors[i].elemLength; ++j) {
                file.read(&c[j], 1);
                s += c[j];
            }
            dir->lump = s;

            cur->childDir.push_back(dir);
        }
    }
    return wad;
}

string Wad::getMagic() {
    string mag(this->magic);
    return mag;
}

bool Wad::isContent(const string &path) {
    Directory* cur = this->root;
    stringstream s(path);
    string dir;

    //Navigates to proper directory based on path
    while(getline(s, dir, '/')) {
        for(unsigned int i = 0; i < cur->childDir.size(); ++i) {
            if(cur->childDir[i]->name == dir) {
                cur = cur->childDir[i];
            }
        }
    }

    if(!cur->isDir) {
        return true;
    }
    return false;
}

bool Wad::isDirectory(const string &path) {
    Directory* cur = this->root;
    stringstream s(path);
    string dir;

    while(getline(s, dir, '/')) {
        for(unsigned int i = 0; i < cur->childDir.size(); ++i) {
            if(cur->childDir[i]->name == dir) {
                cur = cur->childDir[i];
            }
        }
    }

    if(cur->isDir) {
        return true;
    }
    return false;
}

int Wad::getSize(const string &path) {
    if(isContent(path)) {
        Directory* cur = this->root;
        stringstream s(path);
        string dir;

        while(getline(s, dir, '/')) {
            for(unsigned int i = 0; i < cur->childDir.size(); ++i) {
                if(cur->childDir[i]->name == dir) {
                    cur = cur->childDir[i];
                }
            }
        }
        return cur->lump.size();
    }
    return -1;
}

int Wad::getContents(const string &path, char *buffer, int length, int offset) {
    if(isContent(path)) {
        Directory* cur = this->root;
        stringstream s(path);
        string dir;

        while(getline(s, dir, '/')) {
            for(unsigned int i = 0; i < cur->childDir.size(); ++i) {
                if(cur->childDir[i]->name == dir) {
                    cur = cur->childDir[i];
                }
            }
        }

        int buffStart = 0;
        for(int i = offset; i < length; ++i) { //Load lump data into buffer
            buffer[buffStart] = cur->lump[i];
            buffStart++;
        }
        return buffStart;
    }
    return -1;
}

int Wad::getDirectory(const string &path, vector<string> *directory) {
    if(isDirectory(path)) {
        Directory* cur = this->root;
        stringstream s(path);
        string dir;

        while(getline(s, dir, '/')) {
            for(unsigned int i = 0; i < cur->childDir.size(); ++i) {
                if(cur->childDir[i]->name == dir) {
                    cur = cur->childDir[i];
                }
            }
        }

        int numEntries = 0;
        for(unsigned int i = 0; i < cur->childDir.size(); ++i) { //Load entries into vector
            directory->push_back(cur->childDir[i]->name);
            numEntries++;
        }
        return numEntries;
    }
    return -1;
}