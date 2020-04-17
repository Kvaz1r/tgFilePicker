#include <TGUI/TGUI.hpp>
#include <iostream>

#include "FilePicker.h"
#include "SaveFileDialog.h"

class MyFrame
{
public:
    MyFrame();
    void main();

private:
    sf::RenderWindow window;
    tgui::Gui gui;
};

MyFrame::MyFrame()
{
    window.create(sf::VideoMode(800, 600), "FilePicker and SaveFileDialog sample");
    gui.setTarget(window);
    auto panel = tgui::Panel::create();

    auto picker = FilePicker::create();
    picker->getButton()->setText(L"Browse");
    picker->setDir("E:\\");

    panel->add(picker);

    auto button = tgui::Button::create();
    button->setText(L"SaveFileDialog");

    button->setPosition({ tgui::bindLeft(picker->getEditBox()), tgui::bindBottom(picker->getEditBox()) });
    button->connect("pressed", [this, panel]()
        {
            auto ptr = SaveFileDialog::create(*panel);
            ptr->connect("Closed", [this, ptr]()
                {
                    if (ptr->getStatus() == SaveFileDialog::Status::OK)
                    {
                        std::wcout << "Path is " << ptr->getPath().asWideString() << '\n';
                    }
                    else if (ptr->getStatus() == SaveFileDialog::Status::Cancel)
                    {
                        std::wcout << "Cancelled\n";
                    }
                    ptr->destroy();
                });
            gui.add(ptr);
        });
    panel->add(button);

    gui.add(panel);
}

void MyFrame::main()
{
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            else if (event.type == sf::Event::Resized)
            {
                window.setView(sf::View(sf::FloatRect(0.f, 0.f, static_cast<float>(event.size.width), static_cast<float>(event.size.height))));
                gui.setView(window.getView());
            }
            gui.handleEvent(event);
        }

        window.clear();
        gui.draw();
        window.display();
    }
}

int main()
{
    MyFrame().main();
}
