// This is free and unencumbered software released into the public
// domain.  For more information, see <http://unlicense.org> or the
// accompanying UNLICENSE file.

#pragma once
#include <TGUI/TGUI.hpp>

#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <array>
#include <iostream>

#include <windows.h>

class View : public tgui::ListView
{
public:
    typedef std::shared_ptr<View> Ptr;

    View() : tgui::ListView()
    {
        setTextSize(15);
        setExpandLastColumn(true);
    }

    static Ptr create()
    {
        return std::make_shared<View>();
    }

    void load(const tgui::String& val, bool showHiddens)
    {
        removeAllItems();
        auto i = 0;

        addItem({ std::to_string(++i), ".." });

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
        double len = s;
        short order = 0;
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

class OpenFileDialog : public tgui::ChildWindow
{
public:
    typedef std::shared_ptr<OpenFileDialog> Ptr;
    enum class Status { OK, Cancel };

    OpenFileDialog(const sf::String& title = "",
        tgui::String dir = std::filesystem::current_path().generic_wstring(),
        unsigned int tButtons = tgui::ChildWindow::TitleButton::Close) : ChildWindow(title, tButtons),
        m_dir(dir), m_curPath(dir)
    {
        setTitleAlignment(tgui::ChildWindow::TitleAlignment::Center);
        setResizable();

        auto label = tgui::Label::create();
        label->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
        label->setText(dir.asWideString());
        label->setTextSize(14);
        label->setSize(getSize().x, 20);
        add(label);

        auto showHidden = tgui::CheckBox::create("Show hidden files");

        auto listView = View::create();
        listView->setPosition({ tgui::bindLeft(label), tgui::bindBottom(label) + 10 });

        listView->connect("DoubleClicked", [this, listView, label, showHidden](int id)
            {
                auto fname = listView->getItemCell(id, 1).toWideString();
                auto s = std::filesystem::path(m_curPath) / fname;
                if (std::filesystem::is_directory(s))
                {
                    if (fname == L"..")
                    {
                        auto path = std::filesystem::path(m_curPath);

                        if (path == path.root_path())
                        {
                            m_curPath.clear();
                            listView->load();
                            label->setText("");
                            return;
                        }

                        s = path.parent_path().wstring();
                    }
                    m_curPath = s.wstring();
                    label->setText(s.c_str());
                    listView->load(m_curPath, showHidden->isChecked());
                }
            });

        listView->addColumn(L"ID");
        listView->addColumn(L"File");
        listView->addColumn(L"Size");
        listView->addColumn(L"Modified");
        listView->setColumnWidth(0, getSize().x * 0.08f);
        listView->setColumnWidth(1, getSize().x * 0.5f);
        listView->setColumnWidth(2, getSize().x * 0.12f);
        listView->setColumnWidth(3, getSize().x * 0.3f);

        add(listView);

        showHidden->setTextSize(16);
        showHidden->setSize(40, 40);
        showHidden->connect("Changed", [this, listView](bool state)
            {
                listView->load(m_curPath, state);
            });
        listView->load(m_dir, showHidden->isChecked());

        add(showHidden);


        auto select = tgui::Button::create("Select");
        select->setSize(100, 40);
        select->setTextSize(20);
        select->connect("pressed", [this, listView]()
            {
                auto idx = listView->getSelectedItemIndex();
                if (idx == -1)
                {
                    return;
                }

                auto fname = listView->getItemCell(idx, 1).toWideString();
                m_curPath = (std::filesystem::path(m_curPath) / fname).wstring();
                m_status = Status::OK;
                close();
            });

        add(select);

        auto cancel = tgui::Button::create("Cancel");
        cancel->setSize(100, 40);
        cancel->setTextSize(20);
        cancel->connect("pressed", [this]()
            {
                m_curPath = "";
                m_status = Status::Cancel;
                close();
            });

        add(cancel);

        listView->setSize(getSize().x, getSize().y - label->getSize().y - select->getSize().y - 20);
        showHidden->setPosition({ tgui::bindLeft(listView), tgui::bindBottom(listView) + 10 });
        cancel->setPosition({ tgui::bindRight(listView) - cancel->getSize().x, tgui::bindBottom(listView) + 10 });
        select->setPosition({ tgui::bindLeft(cancel) - select->getSize().x - 10, tgui::bindTop(cancel) });

        connect("SizeChanged", [this, listView, select, cancel, label, showHidden]
            {
                listView->setSize(getSize().x, getSize().y - label->getSize().y - select->getSize().y - 20);
                listView->setColumnWidth(0, getSize().x * 0.08f);
                listView->setColumnWidth(1, getSize().x * 0.5f);
                listView->setColumnWidth(2, getSize().x * 0.12f);
                listView->setColumnWidth(3, getSize().x * 0.3f);

                showHidden->setPosition({ tgui::bindLeft(listView), tgui::bindBottom(listView) + 10 });
                cancel->setPosition({ tgui::bindRight(listView) - cancel->getSize().x, tgui::bindBottom(listView) + 10 });
                select->setPosition({ tgui::bindLeft(cancel) - select->getSize().x - 10, tgui::bindTop(cancel) });
            });
    }

    static Ptr create(tgui::Container& c, const sf::String& title = "",
        tgui::String dir = std::filesystem::current_path().generic_wstring(),
        unsigned int tButtons = tgui::ChildWindow::TitleButton::Close)
    {
        auto t = std::make_shared<OpenFileDialog>(title, dir, tButtons);
        c.add(t);
        return t;
    }

    tgui::String getPath() const { return m_curPath; }

    Status getStatus() const { return m_status; }

private:
    tgui::String m_dir;
    tgui::String m_curPath;
    Status m_status = Status::Cancel;
};
