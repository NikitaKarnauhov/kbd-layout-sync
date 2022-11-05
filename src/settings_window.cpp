// vi: ts=4 sw=4 tw=100 et

#include "settings_window.h"
#include "settings.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QFileDialog>

namespace
{

const QString g_keyboard_group_separator = " → ";

}

SettingsWindow::SettingsWindow(OnSettingsChanged on_settings_changed)
    : QDialog{nullptr, Qt::Window | Qt::WindowCloseButtonHint}
    , on_settings_changed_{std::move(on_settings_changed)}

{
    setAttribute(Qt::WA_DeleteOnClose);
    assert(on_settings_changed_);
    Settings settings = load_settings();

    setWindowTitle("Settings — KbLayoutSync");
    setWindowIcon(QIcon(":/kbd-layout-sync.svg"));
    resize(600, 400);

    QVBoxLayout * const vlayout = new QVBoxLayout();

    {
        QGridLayout * const glayout = new QGridLayout();
        QWidget * const glayout_widget = new QWidget();

        glayout_widget->setLayout(glayout);
        vlayout->addWidget(glayout_widget);

        glayout->addWidget(new QLabel("Receiver host"), 0, 0);
        receiver_host_line_edit_ = new QLineEdit(settings.receiver_host.c_str());
        glayout->addWidget(receiver_host_line_edit_, 0, 1);
        glayout->addWidget(new QLabel("Receiver port"), 1, 0);
        receiver_port_line_edit_ = new QLineEdit(settings.receiver_port.c_str());
        glayout->addWidget(receiver_port_line_edit_, 1, 1);
    }

    {
        QGridLayout * const glayout = new QGridLayout();
        QWidget * const glayout_widget = new QWidget();

        glayout_widget->setLayout(glayout);
        vlayout->addWidget(glayout_widget);

        glayout->addWidget(new QLabel("Keyboard groups"), 0, 0, 1, 4);

        keyboard_groups_list_widget_ = new QListWidget();
        glayout->addWidget(keyboard_groups_list_widget_, 1, 0, 1, 4);

        for (const auto & [local, remote] : settings.keyboard_groups)
        {
            keyboard_groups_list_widget_->addItem(QString((local + g_keyboard_group_separator.toStdString() + remote).c_str()));
        }

        glayout->addWidget(new QLabel("Local group"), 2, 0);
        keyboard_group_local_line_edit_ = new QLineEdit("0");
        glayout->addWidget(keyboard_group_local_line_edit_, 2, 1);
        glayout->addWidget(new QLabel("Remote group"), 3, 0);
        keyboard_group_remote_line_edit_ = new QLineEdit("0");
        glayout->addWidget(keyboard_group_remote_line_edit_, 3, 1);
        
        QPushButton * add_button = new QPushButton("Add");
        connect(add_button, &QPushButton::clicked, this, &SettingsWindow::on_add_keyboard_group);
        glayout->addWidget(add_button, 2, 2);

        QPushButton * remove_button = new QPushButton("Remove");
        connect(remove_button, &QPushButton::clicked, this, &SettingsWindow::on_remove_keyboard_group);
        glayout->addWidget(remove_button, 2, 3);
    }

    {
        QGridLayout * const glayout = new QGridLayout();
        QWidget * const glayout_widget = new QWidget();

        glayout_widget->setLayout(glayout);
        vlayout->addWidget(glayout_widget);

        glayout->addWidget(new QLabel("XkbSwitch library location"), 0, 0, 1, 2);
        xkbswitchlib_path_line_edit_ = new QLineEdit(settings.xkbswitchlib_path.string().c_str());
        glayout->addWidget(xkbswitchlib_path_line_edit_, 1, 0);

        QPushButton * browse_button = new QPushButton("Browse");
        connect(browse_button, &QPushButton::clicked, this, &SettingsWindow::on_browse_for_xkbswitchlib);
        glayout->addWidget(browse_button, 1, 1);
    }

    QDialogButtonBox * const button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(button_box, &QDialogButtonBox::accepted, this, &SettingsWindow::on_ok);
    connect(button_box, &QDialogButtonBox::rejected, this, &SettingsWindow::on_cancel);

    vlayout->addWidget(button_box);

    setLayout(vlayout);
}

void SettingsWindow::on_ok()
{
    Settings settings;

    settings.receiver_host = receiver_host_line_edit_->text().toStdString();
    settings.receiver_port = receiver_port_line_edit_->text().toStdString();

    for (std::size_t row = 0; row < keyboard_groups_list_widget_->count(); ++row)
    {
        const QListWidgetItem * const item = keyboard_groups_list_widget_->item(row);
        const QString str = item->text();
        const std::size_t local_length = str.indexOf(g_keyboard_group_separator);
        std::string local = str.left(local_length).trimmed().toStdString();
        std::string remote = str.right(str.size() - local_length - g_keyboard_group_separator.size()).trimmed().toStdString();
        settings.keyboard_groups.emplace(std::move(local), std::move(remote));
    }

    settings.xkbswitchlib_path = xkbswitchlib_path_line_edit_->text().toStdString();
    save_settings(settings);
    on_settings_changed_(settings);

    accept();
}

void SettingsWindow::on_cancel()
{
    reject();
}

void SettingsWindow::on_add_keyboard_group()
{
    const auto local = keyboard_group_local_line_edit_->text();
    const auto remote = keyboard_group_remote_line_edit_->text();
    keyboard_groups_list_widget_->addItem((local + g_keyboard_group_separator + remote));
}

void SettingsWindow::on_remove_keyboard_group()
{
    if (QListWidgetItem * const item = keyboard_groups_list_widget_->currentItem())
    {
        keyboard_groups_list_widget_->takeItem(keyboard_groups_list_widget_->row(item));
    }
}

void SettingsWindow::on_browse_for_xkbswitchlib()
{
    const auto filename = QFileDialog::getOpenFileName(this, "Locate libxkbswitch library");
    if (!filename.isEmpty())
    {
        xkbswitchlib_path_line_edit_->setText(filename);
    }
}
