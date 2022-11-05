// vi: ts=4 sw=4 tw=100 et

#pragma once

#include "settings.h"

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>

using OnSettingsChanged = std::function<void(const Settings &)>;

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(OnSettingsChanged on_settings_changed);

private:
    void on_ok();
    void on_cancel();
    void on_add_keyboard_group();
    void on_remove_keyboard_group();
    void on_browse_for_xkbswitchlib();

private:
    const OnSettingsChanged on_settings_changed_;
    QLineEdit * receiver_host_line_edit_ = nullptr;
    QLineEdit * receiver_port_line_edit_ = nullptr;
    QListWidget * keyboard_groups_list_widget_ = nullptr;
    QLineEdit * keyboard_group_local_line_edit_ = nullptr;
    QLineEdit * keyboard_group_remote_line_edit_ = nullptr;
    QLineEdit * xkbswitchlib_path_line_edit_ = nullptr;
};
