// This is free and unencumbered software released into the public
// domain.  For more information, see <http://unlicense.org> or the
// accompanying UNLICENSE file.

#pragma once
#include <TGUI/TGUI.hpp>

#include <filesystem>

#include <windows.h>

#include <iostream>

class View : public tgui::ListView
{
public:
    typedef std::shared_ptr<View> Ptr;

    View() : tgui::ListView()
    {
        setTextSize(20);
        setExpandLastColumn(true);
    }

    static Ptr create()
    {
        return std::make_shared<View>();
    }

    void load(const tgui::String& val)
    {
        removeAllItems();
        auto i = 0;

        addItem({ std::to_string(++i), ".." });

        for (const auto& entry : std::filesystem::directory_iterator(val.asWideString()))
        {
            addItem({ std::to_string(++i), entry.path().c_str() });
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
};

class OpenFileDialog : public tgui::ChildWindow
{
public:
    typedef std::shared_ptr<OpenFileDialog> Ptr;

    OpenFileDialog(const sf::String& title = "",
        unsigned int tButtons = tgui::ChildWindow::TitleButton::Close) : ChildWindow(title, tButtons)
    {
        setPosition(100, 100);
        setTitleAlignment(tgui::ChildWindow::TitleAlignment::Center);
        setResizable();

        auto listView = View::create();

        listView->connect("DoubleClicked", [this, listView](int id)
            {
                auto s = listView->getItemCell(id, 1).toWideString();
                if (std::filesystem::is_directory(s))
                {
                    if (s == L"..")
                    {
                        auto path = std::filesystem::path(m_curPath);

                        if (path == path.root_path())
                        {
                            m_curPath.clear();
                            listView->load();
                            return;
                        }

                        s = path.parent_path().wstring();
                    }
                    listView->load(s);
                    m_curPath = s;
                }
            });

        listView->setSize(getSize().x, getSize().y * 0.8);

        listView->addColumn(L"   ID   ");
        listView->addColumn(L"   File   ");
        listView->load(m_dir);

        add(listView);

        auto select = tgui::Button::create("Select");
        select->setPosition({
            tgui::bindLeft(listView) + listView->getSize().x / 2 - select->getSize().x / 2,
            tgui::bindBottom(listView) + getSize().y * 0.05 });

        select->setTextSize(20);
        select->connect("pressed", [this, listView]()
            {
                m_curPath = listView->getItemCell(listView->getSelectedItemIndex(), 1).toWideString();
                close();
            });

        add(select);

        connect("SizeChanged", [this, listView, select] 
            {
            listView->setSize(getSize().x, getSize().y * 0.8);
            select->setPosition({
                tgui::bindLeft(listView) + listView->getSize().x / 2 - select->getSize().x / 2,
                tgui::bindBottom(listView) + getSize().y * 0.05 });
            });
    }

    static Ptr create(const sf::String& title = "", unsigned int tButtons = tgui::ChildWindow::TitleButton::Close)
    {
        return std::make_shared<OpenFileDialog>(title, tButtons);
    }

    tgui::String getPath() const { return m_curPath; }

private:
    tgui::String m_dir = std::filesystem::current_path();
    tgui::String m_curPath = m_dir;
};
