#include <dlfcn.h>
#include <link.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <elf-hook.h>

extern "C"
{
    int global_multiply(int x, int y)
    {
        return x * y;
    }

    static int mock_global_multiply(int x, int y)
    {
        return x + y;
    }

    static int static_multiply(int x, int y)
    {
        return x * y;
    }

    static int mock_static_multiply(int x, int y)
    {
        return x + y;
    }

    static int mock_puts(const char *string)
    {
        (void) string;
        return 42;
    }

    static void current_lib_func() 
    {}
}

class ElfHook : public testing::Test
{
public:
    void SetUp() override 
    {
        void *func_address = reinterpret_cast<void*>(&current_lib_func);
        ASSERT_EQ(dl_get_info_by_address(func_address, &_dl_info), 0);
    }

    void TearDown() override {}

protected:
    dl_info_t _dl_info;
};

typedef int (*dep_puts_t)(const char*);

TEST_F(ElfHook, GlobalFunction_ShouldBeHooked)
{
    void *original = nullptr;

    // hook
    original = ELF_HOOK(&_dl_info, "global_multiply", mock_global_multiply);
    ASSERT_NE(original, nullptr);
    ASSERT_EQ(global_multiply(1, 2), 3);
    
    // restore
    original = ELF_HOOK(&_dl_info, "global_multiply", original);
    ASSERT_NE(original, nullptr);
    ASSERT_EQ(global_multiply(1, 2), 2);
}

TEST_F(ElfHook, LibcFunction_ShouldBeHooked)
{
    void *original = nullptr;

    // hook
    original = ELF_HOOK(&_dl_info, "puts", mock_puts);
    ASSERT_NE(original, nullptr);
    ASSERT_EQ(puts("Hello, world!"), 42);

    // restore
    original = ELF_HOOK(&_dl_info, "puts", original);
    ASSERT_NE(original, nullptr);
    ASSERT_EQ(puts("Hello, world!"), 14);
}

TEST_F(ElfHook, DlFunction_RTLD_LAZY_ShouldBeHooked)
{
    void *original = nullptr;

    void *handle = dlopen("libdep.so", RTLD_LAZY);
    ASSERT_NE(handle, nullptr);

    dep_puts_t dep_puts = (dep_puts_t)dlsym(handle, "dep_puts");
    ASSERT_NE(dep_puts, nullptr);

    dl_info_t dl_info;
    ASSERT_EQ(dl_get_info_by_handle(handle, &dl_info), 0);

    original = ELF_HOOK(&dl_info, "puts", mock_puts);
    ASSERT_NE(original, nullptr);
    ASSERT_EQ(dep_puts("hello"), 42);
    
    original = ELF_HOOK(&dl_info, "puts", original);
    ASSERT_NE(original, nullptr);
    ASSERT_EQ(dep_puts("hello"), 6);

    ASSERT_EQ(dlclose(handle), 0);
}

TEST_F(ElfHook, DlFunction_RTLD_NOW_ShouldBeHooked)
{
    void *original = nullptr;

    void *handle = dlopen("libdep.so", RTLD_NOW);
    ASSERT_NE(handle, nullptr);

    dep_puts_t dep_puts = (dep_puts_t)dlsym(handle, "dep_puts");
    ASSERT_NE(dep_puts, nullptr);
    
    dl_info_t dl_info;
    ASSERT_EQ(dl_get_info_by_handle(handle, &dl_info), 0);

    original = ELF_HOOK(&dl_info, "puts", mock_puts);
    ASSERT_NE(original, nullptr);
    ASSERT_EQ(dep_puts("hello"), 42);
    
    original = ELF_HOOK(&dl_info, "puts", original);
    ASSERT_NE(original, nullptr);
    ASSERT_EQ(dep_puts("hello"), 6);

    ASSERT_EQ(dlclose(handle), 0);
}

TEST_F(ElfHook, StaticFunction_ShouldNotBeHooked)
{
    void* original = ELF_HOOK(&_dl_info, "static_multiply", mock_static_multiply);
    ASSERT_EQ(original, nullptr);
    ASSERT_EQ(static_multiply(1, 2), 2);
}
