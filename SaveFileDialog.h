// This is free and unencumbered software released into the public
// domain.  For more information, see <http://unlicense.org> or the
// accompanying UNLICENSE file.

#ifndef _TG_SAVEFILEDIALOG_H_
#define _TG_SAVEFILEDIALOG_H_

#include "FilesystemViewer.h"

class SaveFileDialog : public tgui::ChildWindow
{
public:
    typedef std::shared_ptr<SaveFileDialog> Ptr;
    enum class Status { OK, Cancel };

    SaveFileDialog(const sf::String& title,
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

        auto listView = FilesystemViewer::create();
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

        auto fileBox = tgui::EditBox::create();
        fileBox->setDefaultText("Enter filename");
        fileBox->setSize(getSize().x, 20);

        listView->connect("ItemSelected", [this, listView, fileBox](int idx)
            {
                auto fname = listView->getItemCell(idx, 1).toWideString();
                auto s = std::filesystem::path(m_curPath) / fname;
                if (!std::filesystem::is_directory(s))
                {
                    fileBox->setText(listView->getItemCell(idx, 1));
                }
            });

        add(fileBox);

        showHidden->setTextSize(14);
        showHidden->setSize(25, 25);
        showHidden->connect("Changed", [this, listView](bool state)
            {
                listView->load(m_curPath, state);
            });
        listView->load(m_dir, showHidden->isChecked());

        add(showHidden);

        auto select = tgui::Button::create("Save");
        select->setSize(100, 25);
        select->setTextSize(20);
        select->connect("pressed", [this, fileBox]()
            {
                auto fname = fileBox->getText().toWideString();
                if (fname.empty())
                {
                    return;
                }

                m_curPath = (std::filesystem::path(m_curPath) / fname).wstring();
                std::replace(m_curPath.begin(), m_curPath.end(), '/', '\\');
                m_status = Status::OK;
                close();
            });

        add(select);

        auto cancel = tgui::Button::create("Cancel");
        cancel->setSize(100, 25);
        cancel->setTextSize(20);
        cancel->connect("pressed", [this]()
            {
                m_curPath = "";
                m_status = Status::Cancel;
                close();
            });

        add(cancel);

        listView->setSize(getSize().x, getSize().y - label->getSize().y - select->getSize().y - fileBox->getSize().y - 35);
        fileBox->setPosition({ tgui::bindLeft(listView), tgui::bindBottom(listView) + 10 });
        showHidden->setPosition({ tgui::bindLeft(listView), tgui::bindBottom(fileBox) + 10 });
        cancel->setPosition({ tgui::bindRight(listView) - cancel->getSize().x, tgui::bindBottom(fileBox) + 10 });
        select->setPosition({ tgui::bindLeft(cancel) - select->getSize().x - 10, tgui::bindTop(cancel) });

        connect("SizeChanged", [this, listView, select, cancel, label, showHidden, fileBox]
            {
                auto width = getSize().x;
                fileBox->setSize(width, 20);
                listView->setSize(width, getSize().y - label->getSize().y - select->getSize().y - fileBox->getSize().y - 35);

                listView->setColumnWidth(0, width * 0.08f);
                listView->setColumnWidth(1, width * 0.5f);
                listView->setColumnWidth(2, width * 0.12f);
                listView->setColumnWidth(3, width * 0.3f);

                fileBox->setPosition({ tgui::bindLeft(listView), tgui::bindBottom(listView) + 10 });
                showHidden->setPosition({ tgui::bindLeft(listView), tgui::bindBottom(fileBox) + 10 });
                cancel->setPosition({ tgui::bindRight(listView) - cancel->getSize().x, tgui::bindBottom(fileBox) + 10 });
                select->setPosition({ tgui::bindLeft(cancel) - select->getSize().x - 10, tgui::bindTop(cancel) });
            });
    }

    static Ptr create(tgui::Container& c, const sf::String& title = "SaveFileDialog",
        tgui::String dir = std::filesystem::current_path().generic_wstring(),
        unsigned int tButtons = tgui::ChildWindow::TitleButton::Close)
    {
        auto t = std::make_shared<SaveFileDialog>(title, dir, tButtons);
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

#endif
