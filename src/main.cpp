// vi: ts=4 sw=4 tw=100 et

#include "config.h"
#include "settings.h"
#include "settings_window.h"
#include "status.h"
#include "worker.h"
#include "xkb_switch_lib.h"
#include "listener.h"

#if HAS_X11
#include "sender.h"
#endif

#include <memory>
#include <string>
#include <iostream>
#include <mutex>

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QSystemTrayIcon>

class Application
{
public:
    template<typename... Args>
    Application(Args &&... args);

    ~Application();

    int exec();

private:
    QIcon make_icon() const;
    std::unique_ptr<Listener> make_listener(const Settings & settings);

    void start_listener();
    void stop();
    void quit();
    void show_settings();
    void apply_settings(const Settings & settings);

#if HAS_X11
    std::unique_ptr<Sender> make_sender(const Settings & settings);
    void start_sender();
#endif

private:
    QApplication qapplication_;
    QAction * const start_listener_action_;
    QAction * const stop_action_;
    QAction * const settings_action_;
    QAction * const quit_action_;
    QSystemTrayIcon systray_icon_{make_icon()};

    std::mutex mutex_;
    Settings settings_;
    std::unique_ptr<Listener> listener_;

#if HAS_X11
    QAction * const start_sender_action_;
    std::unique_ptr<Sender> sender_;
#endif
};

template<typename... Args>
Application::Application(Args &&... args)
    : qapplication_{std::forward<Args>(args)...}
    , start_listener_action_{new QAction("Start &receiver", &qapplication_)}
    , stop_action_{new QAction("S&top", &qapplication_)}
    , settings_action_{new QAction("&Settings", &qapplication_)}
    , quit_action_{new QAction("&Quit", &qapplication_)}
    , settings_{load_settings()}
    , listener_{make_listener(settings_)}
#if HAS_X11
    , start_sender_action_{new QAction("Start &transmitter", &qapplication_)}
    , sender_{make_sender(settings_)}
#endif
{
    qapplication_.setQuitOnLastWindowClosed(false);
    qapplication_.connect(start_listener_action_, &QAction::triggered, [this] { start_listener(); });
    qapplication_.connect(stop_action_, &QAction::triggered, [this] { stop(); });
    qapplication_.connect(settings_action_, &QAction::triggered, [this] { show_settings(); });
    qapplication_.connect(quit_action_, &QAction::triggered, [this] { quit(); });

#if HAS_X11
    qapplication_.connect(start_sender_action_, &QAction::triggered, [this] { start_sender(); });
#endif

    const auto menu = new QMenu();
    menu->addAction(start_listener_action_);

#if HAS_X11
    menu->addAction(start_sender_action_);
#endif

    menu->addAction(stop_action_);
    menu->addAction(quit_action_);
    menu->addAction(settings_action_);

    systray_icon_.setContextMenu(menu);
    systray_icon_.show();
}

Application::~Application()
{
    const std::scoped_lock lock{mutex_};
    listener_->stop();
#if HAS_X11
    sender_->stop();
#endif
}

int Application::exec()
{
    return qapplication_.exec();
}

QIcon Application::make_icon() const
{
#if ICON_IS_MASK
    QIcon icon = QIcon(":/kbd-layout-sync-mask.svg");
    icon.setIsMask(true);
#else
    QIcon icon = QIcon(":/kbd-layout-sync.svg");
#endif
    return icon;
}

std::unique_ptr<Listener> Application::make_listener(const Settings & settings)
{
    return std::make_unique<Listener>(
        settings_.receiver_host,
        settings_.receiver_port,
        [lib = std::make_shared<XkbSwitchLib>(settings.xkbswitchlib_path)](const std::string layout)
        {
            lib->set_layout(layout);
        });
}

void Application::start_listener()
{
    const std::scoped_lock lock{mutex_};
    listener_->start();
}

void Application::stop()
{
    const std::scoped_lock lock{mutex_};
    listener_->stop();
#if HAS_X11
    sender_->stop();
#endif
}

void Application::quit()
{
    {
        const std::scoped_lock lock{mutex_};
        listener_->stop();
#if HAS_X11
        sender_->stop();
#endif
    }

    qapplication_.quit();
}

void Application::show_settings()
{
    SettingsWindow * const settings_window = new SettingsWindow(
        [this](const Settings & settings)
        {
            apply_settings(settings);
        });

    settings_window->show();
    settings_window->raise();
    settings_window->activateWindow();
}

void Application::apply_settings(const Settings & settings)
{
    const std::scoped_lock lock{mutex_};
    const bool is_listener_running = listener_->status() == Status::Running;

#if HAS_X11
    const bool is_sender_running = sender_->status() == Status::Running;
    sender_->stop();
#endif

    settings_ = settings;
    listener_->stop();

    listener_ = make_listener(settings_);
    if (is_listener_running)
    {
        listener_->start();
    }

#if HAS_X11
    sender_ = make_sender(settings_);
    if (is_sender_running)
    {
        sender_->start();
    }
#endif
}

#if HAS_X11
std::unique_ptr<Sender> Application::make_sender(const Settings & settings)
{
    return std::make_unique<Sender>(
        settings_.receiver_host,
        settings_.receiver_port,
        settings_.keyboard_groups);
}

void Application::start_sender()
{
    const std::scoped_lock lock{mutex_};
    listener_->stop();
    sender_->start();
}
#endif

int main(int argc, char * argv[])
{
    Application application(argc, argv);
    return application.exec();
}
