/*
    Copyright (c) 2017 Christian Mollekopf <mollekopf@kolabsys.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include <signal.h>
#include <execinfo.h>
#include <csignal>
#include <iostream>
#include <cstdlib>
#include <cxxabi.h>
#include <dlfcn.h>
#include <ostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <unistd.h>

#include <QApplication>
#include <QQmlApplicationEngine>

#include <QStandardPaths>
#include <KPackage/PackageLoader>
#include <QQuickImageProvider>
#include <QIcon>
#include <QtWebEngine>

#include <QDebug>

//Print a demangled stacktrace
void printStacktrace()
{
    int skip = 1;
    void *callstack[128];
    const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
    char buf[1024];
    int nFrames = backtrace(callstack, nMaxFrames);
    char **symbols = backtrace_symbols(callstack, nFrames);

    std::ostringstream trace_buf;
    for (int i = skip; i < nFrames; i++) {
        // printf("%s\n", symbols[i]);
        Dl_info info;
        if (dladdr(callstack[i], &info) && info.dli_sname) {
            char *demangled = NULL;
            int status = -1;
            if (info.dli_sname[0] == '_') {
                demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
            }
            snprintf(buf, sizeof(buf), "%-3d %*p %s + %zd\n",
                    i, int(2 + sizeof(void*) * 2), callstack[i],
                    status == 0 ? demangled :
                    info.dli_sname == 0 ? symbols[i] : info.dli_sname,
                    (char *)callstack[i] - (char *)info.dli_saddr);
            free(demangled);
        } else {
            snprintf(buf, sizeof(buf), "%-3d %*p %s\n",
                    i, int(2 + sizeof(void*) * 2), callstack[i], symbols[i]);
        }
        trace_buf << buf;
    }
    free(symbols);
    if (nFrames == nMaxFrames) {
        trace_buf << "[truncated]\n";
    }
    std::cerr << trace_buf.str();
}

static int sCounter = 0;

void crashHandler(int signal)
{
    //Guard against crashing in here
    if (sCounter > 1) {
        std::_Exit(EXIT_FAILURE);
    }
    sCounter++;

    if (signal == SIGABRT) {
        std::cerr << "SIGABRT received\n";
    } else if (signal == SIGSEGV) {
        std::cerr << "SIGSEV received\n";
    } else {
        std::cerr << "Unexpected signal " << signal << " received\n";
    }

    printStacktrace();

    std::fprintf(stdout, "Sleeping for 10s to attach a debugger: gdb attach %i\n", getpid());
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // std::system("exec gdb -p \"$PPID\" -ex \"thread apply all bt\"");
    // This only works if we actually have xterm and X11 available
    // std::system("exec xterm -e gdb -p \"$PPID\"");

    std::_Exit(EXIT_FAILURE);
}

void terminateHandler()
{
    // std::exception_ptr exptr = std::current_exception();
    // if (exptr != 0)
    // {
    //     // the only useful feature of std::exception_ptr is that it can be rethrown...
    //     try {
    //         std::rethrow_exception(exptr);
    //     } catch (std::exception &ex) {
    //         std::fprintf(stderr, "Terminated due to exception: %s\n", ex.what());
    //     } catch (...) {
    //         std::fprintf(stderr, "Terminated due to unknown exception\n");
    //     }
    // } else {
        std::fprintf(stderr, "Terminated due to unknown reason.\n");
    // }
    std::abort();
}

class KubeImageProvider : public QQuickImageProvider
{
public:
    KubeImageProvider()
        : QQuickImageProvider(QQuickImageProvider::Pixmap)
    {
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) Q_DECL_OVERRIDE
    {
        //The platform theme plugin can overwrite our setting again once it gets loaded,
        //so we check on every icon load request...
        if (QIcon::themeName() != "kube") {
            QIcon::setThemeName("kube");
        }
        const auto icon = QIcon::fromTheme(id);
        auto expectedSize = requestedSize;
        //Get the largest size that is still smaller or equal than requested
        //Except if we only have larger sizes, then just pick the closest one
        bool first = true;
        for (const auto s : icon.availableSizes()) {
            if (first && s.width() > requestedSize.width()) {
                expectedSize = s;
                break;
            }
            first = false;
            if (s.width() <= requestedSize.width()) {
                expectedSize = s;
            }
        }
        const auto pixmap = icon.pixmap(expectedSize);
        if (size) {
            *size = pixmap.size();
        }
        return pixmap;
    }
};

int main(int argc, char *argv[])
{

    std::signal(SIGSEGV, crashHandler);
    std::signal(SIGABRT, crashHandler);
    std::set_terminate(terminateHandler);

    QApplication app(argc, argv);
    app.setFont(QFont{"Noto Sans", app.font().pointSize(), QFont::Normal});

    QtWebEngine::initialize();
    QIcon::setThemeName("kube");

    auto package = KPackage::PackageLoader::self()->loadPackage("KPackage/GenericQML", "org.kube.components.kube");
    Q_ASSERT(package.isValid());
    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("kube"), new KubeImageProvider);
    engine.load(QUrl::fromLocalFile(package.filePath("mainscript")));
    return app.exec();
}

