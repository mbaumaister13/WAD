#include <iostream>
#include "Wad.h"
#include <queue>

using namespace std;

void LevelOrderTraversal(Directory* root)
{
    if (root==NULL)
        return;

    // Standard level order traversal code 
    // using queue 
    queue<Directory *> q;  // Create a queue 
    q.push(root); // Enqueue root  
    while (!q.empty())
    {
        int n = q.size();

        // If this node has children 
        while (n > 0)
        {
            // Dequeue an item from queue and print it 
            Directory * p = q.front();
            q.pop();
            cout << p->name << " ";

            // Enqueue all children of the dequeued item 
            for (unsigned int i=0; i<p->childDir.size(); i++)
                q.push(p->childDir[i]);
            n--;
        }

        cout << endl; // Print new line between two levels 
    }
}

void exploreDirectory(Wad *data, const string path, int level)
{
    for (int index = 0; index < level; index++)
        cout << " ";

    vector<string> entries;
    cout << "[Objects at this level:" << data->getDirectory(path, &entries) << "]" << endl;

    for (string entry : entries)
    {
        string entryPath = path + entry;

        for (int index = 0; index < level; index++)
            cout << " ";

        if (data->isDirectory(entryPath))
        {
            cout << level << ". DIR: " << entry << endl;
            exploreDirectory(data, entryPath + "/", level + 1);
        }
        else if (data->isContent(entryPath))
            cout << level << ". CONTENT: " << entry << "; Size: " << data->getSize(entryPath) << endl;
        else
            cout << "***WARNING: entry " << entry << " has invalid type!***" << endl;
    }
}

void exploreDirectory(Wad *data, const string path)
{
    cout << "EXPLORING: " << path << endl;
    exploreDirectory(data, path, 1);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        cout << "No file specified. Exiting." << endl;
        exit(EXIT_SUCCESS);
    }

    Wad *myWad = Wad::loadWad(argv[1]);
    if (argc > 2)
    {
        //cout << myWad->testPath(argv[2]);
        int dataSize = myWad->getSize(argv[2]);
        char *myData = new char[dataSize + 1];
        myData[dataSize] = 0;
        myWad->getContents(argv[2], myData, dataSize, 0);
        cout << myData << endl;
    }
    else
        exploreDirectory(myWad, "/");
}
