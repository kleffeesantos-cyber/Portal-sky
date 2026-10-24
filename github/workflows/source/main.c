#include <Common.h>
#include <GoldHEN.h>
#include <orbis/libkernel.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

attr_public const char *g_pluginName = "portal_log";
attr_public const char *g_pluginDesc = "Registra modulos carregados pelo jogo";
attr_public const char *g_pluginAuth = "usuario";
attr_public uint32_t g_pluginVersion = 0x00000100;

#define LOG_PATH "/data/GoldHEN/portal_log.txt"

static void log_line(const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf) - 2, fmt, ap);
    va_end(ap);
    if (n < 0) return;
    if (n > (int)sizeof(buf) - 2) n = sizeof(buf) - 2;
    buf[n++] = '\n';
    buf[n] = 0;

    final_printf("[portal_log] %s", buf);

    int fd = sceKernelOpen(LOG_PATH, 0x1 | 0x8 | 0x200, 0777);
    if (fd >= 0) {
        sceKernelWrite(fd, buf, n);
        sceKernelClose(fd);
    }
}

static int has_usb(const char *name)
{
    return strstr(name, "Usb") || strstr(name, "usb") || strstr(name, "USB");
}

static void dump_modules(int round)
{
    OrbisKernelModule list[256];
    size_t count = 0;
    int ret = sceKernelGetModuleList(list, 256, &count);
    log_line("--- rodada %d: ret=0x%08x, %d modulos ---", round, ret, (int)count);
    if (ret != 0) return;

    for (size_t i = 0; i < count; i++) {
        OrbisKernelModuleInfo info;
        memset(&info, 0, sizeof(info));
        info.size = sizeof(info);
        if (sceKernelGetModuleInfo(list[i], &info) == 0) {
            log_line("%s%s", has_usb(info.name) ? "[USB] " : "", info.name);
        }
    }
}

static void *log_thread(void *arg)
{
    (void)arg;
    for (int round = 1; round <= 12; round++) {
        sceKernelUsleep(10 * 1000 * 1000);
        dump_modules(round);
    }
    log_line("--- fim do log ---");
    return NULL;
}

int32_t attr_module_hidden attr_public plugin_load(int32_t argc, const char *argv[])
{
    (void)argc;
    (void)argv;
    final_printf("[GoldHEN] <%s\\Ver.0x%08x> %s\n", g_pluginName, g_pluginVersion, __func__);
    log_line("=== portal_log iniciado ===");

    OrbisPthread thr;
    int ret = scePthreadCreate(&thr, NULL, log_thread, NULL, "portal_log");
    log_line("scePthreadCreate=0x%08x", ret);
    return 0;
}

int32_t attr_module_hidden attr_public plugin_unload(int32_t argc, const char *argv[])
{
    (void)argc;
    (void)argv;
    final_printf("[GoldHEN] <%s\\Ver.0x%08x> %s\n", g_pluginName, g_pluginVersion, __func__);
    return 0;
}
