#include "settings.h"

#include <QDir>
#include <QSettings>
#include <QStandardPaths>

namespace
{

QSettings make_qsettings()
{
    const QString config_path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    assert(!config_path.isEmpty());
    return QSettings(QDir(config_path).filePath("kb-layout-sync.ini"), QSettings::IniFormat);
}

}

Settings load_settings()
{
    QSettings qsettings = make_qsettings();
    Settings result;

    result.receiver_host = qsettings.value("receiver_host", "0.0.0.0").toString().toStdString();
    result.receiver_port = qsettings.value("receiver_port", "36032").toString().toStdString();

    const int group_count = qsettings.beginReadArray("keyboard_groups");
    for (int i = 0; i < group_count; ++i) {
        qsettings.setArrayIndex(i);
        std::string local = qsettings.value("local", "0").toString().toStdString();
        std::string remote = qsettings.value("remote", "0").toString().toStdString();
        result.keyboard_groups.emplace(std::move(local), std::move(remote));
    }
    qsettings.endArray();

    result.xkbswitchlib_path =
        qsettings.value("xkbswitchlib_path", "/usr/loca/lib/libxkbswitch.so").toString().toStdString();

    return result;
}

void save_settings(const Settings & settings)
{
    QSettings qsettings = make_qsettings();

    qsettings.setValue("receiver_host", QString(settings.receiver_host.c_str()));
    qsettings.setValue("receiver_port", QString(settings.receiver_port.c_str()));

    qsettings.beginWriteArray("keyboard_groups");
    auto keyboard_group_it = settings.keyboard_groups.begin();
    for (std::size_t i = 0; i < settings.keyboard_groups.size(); ++i, ++keyboard_group_it)
    {
        qsettings.setArrayIndex(i);
        qsettings.setValue("local", QString(keyboard_group_it->first.c_str()));
        qsettings.setValue("remote", QString(keyboard_group_it->second.c_str()));
    }
    qsettings.endArray();

    qsettings.setValue("xkbswitchlib_path", QString(settings.xkbswitchlib_path.string().c_str()));
}
