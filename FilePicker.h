// This is free and unencumbered software released into the public
// domain.  For more information, see <http://unlicense.org> or the
// accompanying UNLICENSE file.

#pragma once
#include <TGUI/TGUI.hpp>

#include <filesystem>

#include <windows.h>

class View : public tgui::ListView
{
public:
    typedef std::shared_ptr<View> Ptr;

    View() : tgui::ListView()
    {
        setTextSize(20);
        setExpandLastColumn(true);
    }

    static View::Ptr create()
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

class FilePicker : public tgui::Group
{
public:
    typedef std::shared_ptr<FilePicker> Ptr;

    FilePicker(const tgui::Layout2d& size = { "100%", "100%" }) : Group(size)
    {
        m_Browse = tgui::Button::create();
        m_Browse->setText("Browse");
        m_Browse->connect("pressed", [this]()
            {
                auto ptr = tgui::ChildWindow::create(L"File ");
                ptr->setPosition(100, 100);
                ptr->setTitleAlignment(tgui::ChildWindow::TitleAlignment::Center);
                ptr->setResizable();

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

                listView->setSize(ptr->getSize().x, ptr->getSize().y * 0.8);

                listView->addColumn(L"   ID   ");
                listView->addColumn(L"   File   ");
                listView->load(m_dir);

                ptr->add(listView);

                auto select = tgui::Button::create("Select");
                select->setPosition({
                    tgui::bindLeft(listView) + listView->getSize().x / 2 - select->getSize().x / 2,
                    tgui::bindBottom(listView) + ptr->getSize().y * 0.05 });

                select->setTextSize(20);
                select->connect("pressed", [this, listView, ptr]()
                    {
                        m_Path->setText(listView->getItemCell(listView->getSelectedItemIndex(), 1));
                        ptr->close();
                    });

                ptr->add(select);

                ptr->connect("SizeChanged", [ptr, listView, select] {
                    listView->setSize(ptr->getSize().x, ptr->getSize().y * 0.8);
                    select->setPosition({
                        tgui::bindLeft(listView) + listView->getSize().x / 2 - select->getSize().x / 2,
                        tgui::bindBottom(listView) + ptr->getSize().y * 0.05 });
                    });

                add(ptr);
            });

        m_Path = tgui::EditBox::create();
        m_Path->setReadOnly();

        add(m_Browse);
        add(m_Path);
        m_Browse->setPosition({ tgui::bindRight(m_Path) + 20, tgui::bindTop(m_Path) });
    }

    static Ptr FilePicker::create(const tgui::Layout2d& size = { "100%", "100%" })
    {
        return std::make_shared<FilePicker>(size);
    }

    tgui::Button::Ptr getButton() { return m_Browse; }

    tgui::EditBox::Ptr getEditBox() { return m_Path; }

    tgui::String getPath() const { return tgui::String(m_Path->getText()); }

    tgui::String getDir() const { return m_dir; }

    tgui::String setDir(const tgui::String& dir) { m_dir = dir; }

protected:
    tgui::Button::Ptr m_Browse;
    tgui::EditBox::Ptr m_Path;

private:
    tgui::String m_dir = std::filesystem::current_path();
    tgui::String m_curPath = m_dir;
};
