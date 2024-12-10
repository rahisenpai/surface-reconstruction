// filesystem.cpp
#include "filesystem.h"
#include <stdexcept>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

FileSystem::FileSystem(const std::string& dataPath) 
    : m_dataPath(dataPath), m_currentIndex(0) {
    if (!fs::exists(dataPath)) {
        throw std::runtime_error("Data directory not found: " + dataPath);
    }
    
    // Load file list
    m_files = getContourFiles();
    if (m_files.empty()) {
        throw std::runtime_error("No contour files found in: " + dataPath);
    }
    
    // Load initial file
    loadCurrentFile();
}

void FileSystem::nextFile() {
    if (!m_files.empty()) {
        m_currentIndex = (m_currentIndex + 1) % m_files.size();
        loadCurrentFile();
    }
}

void FileSystem::previousFile() {
    if (!m_files.empty()) {
        m_currentIndex = (m_currentIndex - 1 + m_files.size()) % m_files.size();
        loadCurrentFile();
    }
}

bool FileSystem::selectFile(size_t index) {
    if (index < m_files.size()) {
        m_currentIndex = index;
        loadCurrentFile();
        return true;
    }
    return false;
}

void FileSystem::loadCurrentFile() {
    if (!m_files.empty()) {
        m_currentContours = loadContourFile(m_files[m_currentIndex]);
    }
}

std::vector<std::string> FileSystem::getContourFiles() const {
    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(m_dataPath)) {
        if (entry.path().extension() == ".contour") {
            files.push_back(entry.path().filename().string());
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

std::vector<ContourPlane> FileSystem::loadContourFile(const std::string& filename) const {
    std::string fullPath = m_dataPath + "/" + filename;
    return parseContourFile(fullPath);
}