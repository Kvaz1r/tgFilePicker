// This is free and unencumbered software released into the public
// domain.  For more information, see <http://unlicense.org> or the
// accompanying UNLICENSE file.

#ifndef _TG_FILEPICKER_H_
#define _TG_FILEPICKER_H_

#include "OpenFileDialog.h"

class FilePicker : public tgui::Group
{
public:
    typedef std::shared_ptr<FilePicker> Ptr;

    FilePicker(const tgui::Layout2d& size = { "100%", "100%" }) : Group(size)
    {
        m_Browse = tgui::Button::create();
        m_Browse->setText("Browse");
        m_Browse->onPress([this]()
            {
                auto ptr = OpenFileDialog::create(*this, L"Open file dialog", m_dir);
                ptr->onClose([this, ptr]()
                    {
                        if (ptr->getStatus() == OpenFileDialog::Status::OK)
                        {
                            m_Path->setText(ptr->getPath().toWideString());
                        }
                        ptr->destroy();
                    });
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

    void setDir(const tgui::String& dir) { m_dir = dir; }

protected:
    tgui::Button::Ptr m_Browse;
    tgui::EditBox::Ptr m_Path;

private:
    tgui::String m_dir = std::filesystem::current_path();
};

#endif