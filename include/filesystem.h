// filesystem.h
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <vector>
#include <filesystem>
#include "contour.h"

class FileSystem {
public:
    FileSystem(const std::string& dataPath = "../data");
    
    // File management
    std::vector<std::string> getContourFiles() const;
    std::vector<ContourPlane> loadContourFile(const std::string& filename) const;
    std::string getDataPath() const { return m_dataPath; }
    
    // File navigation
    void nextFile();
    void previousFile();
    bool selectFile(size_t index);
    std::vector<ContourPlane> getCurrentContours() const { return m_currentContours; }
    std::string getCurrentFileName() const { return m_files[m_currentIndex]; }
    size_t getCurrentIndex() const { return m_currentIndex; }
    size_t getFileCount() const { return m_files.size(); }

private:
    std::string m_dataPath;
    std::vector<std::string> m_files;
    size_t m_currentIndex;
    std::vector<ContourPlane> m_currentContours;
    void loadCurrentFile();
};

#endif