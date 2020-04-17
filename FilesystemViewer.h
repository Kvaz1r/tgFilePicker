// This is free and unencumbered software released into the public
// domain.  For more information, see <http://unlicense.org> or the
// accompanying UNLICENSE file.

#ifndef _TG_FILESYSTEM_VIEWER_H_
#define _TG_FILESYSTEM_VIEWER_H_

#include <TGUI/TGUI.hpp>

#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <array>
#include <iostream>

#include <windows.h>

class FilesystemViewer : public tgui::ListView
{
public:
    typedef std::shared_ptr<FilesystemViewer> Ptr;

    FilesystemViewer() : tgui::ListView()
    {
        setTextSize(15);
        setExpandLastColumn(true);
    }

    static Ptr create()
    {
        return std::make_shared<FilesystemViewer>();
    }

    void load(const tgui::String& val, bool showHiddens)
    {
        removeAllItems();
        auto i = 0;

        addItem({ std::to_string(++i), ".." });

        try
        {
            for (const auto& entry : std::filesystem::directory_iterator(val.asWideString()))
            {
                try
                {
                    if (!showHiddens && (::GetFileAttributes(entry.path().c_str()) & FILE_ATTRIBUTE_HIDDEN))
                        continue;

                    if (std::filesystem::is_regular_file(entry))
                    {
                        auto ft = entry.last_write_time();
                        auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()
                            + (ft - std::filesystem::file_time_type::clock::now()));

                        std::stringstream sbuf;
                        sbuf << std::put_time(localtime(&t), "%F %H:%M:%S");

                        addItem({ std::to_string(++i), entry.path().filename().c_str(),
                            get_size(entry.file_size()), sbuf.str() });
                    }
                    else
                    {
                        addItem({ std::to_string(++i), entry.path().filename().c_str() });
                    }
                }
                catch (const std::filesystem::filesystem_error & e)
                {
                    std::wcerr << e.what() << '\n';
                }
            }
        }
        catch (const std::filesystem::filesystem_error & e)
        {
            std::wcerr << e.what() << '\n';
        }
    }

    void load()
    {
        removeAllItems();
        wchar_t LogicalDrives[MAX_PATH] = { 0 };

        if (::GetLogicalDriveStringsW(MAX_PATH, LogicalDrives))
        {
            wchar_t* drive = LogicalDrives;
            auto i = 0;
            while (*drive)
            {
                addItem({ std::to_string(++i), drive });
                drive += wcslen(drive) + 1;
            }
        }
    }

private:
    static std::string get_size(uintmax_t s)
    {
        std::array<std::string, 5> sizes = { "B", "KB", "MB", "GB", "TB" };
        uintmax_t len = s;
        unsigned short order = 0;
        while (len >= 1024 && order < sizes.size() - 1)
        {
            order++;
            len = len / 1024;
        }

        std::stringstream buf;
        buf << std::setprecision(3) << len << sizes[order];
        return buf.str();
    }
};

#endif