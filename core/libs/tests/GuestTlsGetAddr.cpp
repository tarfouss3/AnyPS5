#include "prx/libc/include/general/VabiMacros.hpp"
#include <link.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <thread>

extern "C" void* APS5_VABI __tls_get_addr_nid_postfix(void* entry);

struct TlsIndex {
    std::uint64_t module;
    std::uint64_t offset;
};

static thread_local int tlsFirst = 11;
static thread_local int tlsSecond = 22;

static void Require(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "TLS check failed at line %d\n", line);
        std::abort();
    }
}
#define Check(value) Require((value), __LINE__)

struct Search {
    std::uintptr_t code;
    std::uint64_t module;
    std::uintptr_t block;
};

static int FindExecutable(dl_phdr_info* image, std::size_t, void* data) {
    auto& search = *static_cast<Search*>(data);
    for (std::uint16_t i = 0; i < image->dlpi_phnum; ++i) {
        const auto& header = image->dlpi_phdr[i];
        if (header.p_type != PT_LOAD) continue;
        const std::uintptr_t start = image->dlpi_addr + header.p_vaddr;
        if (search.code < start || search.code - start >= header.p_memsz) continue;
        search.module = image->dlpi_tls_modid;
        search.block = reinterpret_cast<std::uintptr_t>(image->dlpi_tls_data);
        return 1;
    }
    return 0;
}

static Search Locate() {
    Search search{reinterpret_cast<std::uintptr_t>(&FindExecutable), 0, 0};
    Check(dl_iterate_phdr(FindExecutable, &search) == 1);
    Check(search.module != 0 && search.block != 0);
    return search;
}

static TlsIndex IndexOf(const int& variable) {
    const auto search = Locate();
    return {search.module, reinterpret_cast<std::uintptr_t>(&variable) - search.block};
}

int main() {
    TlsIndex first = IndexOf(tlsFirst);
    TlsIndex second = IndexOf(tlsSecond);
    Check(first.offset != second.offset);
    Check(__tls_get_addr_nid_postfix(&first) == &tlsFirst);
    Check(__tls_get_addr_nid_postfix(&second) == &tlsSecond);
    tlsFirst = 33;
    const void* mainFirst = &tlsFirst;
    std::thread worker([&] {
        Check(__tls_get_addr_nid_postfix(&first) == &tlsFirst);
        Check(__tls_get_addr_nid_postfix(&second) == &tlsSecond);
        Check(&tlsFirst != mainFirst);
        Check(*static_cast<int*>(__tls_get_addr_nid_postfix(&first)) == 11);
        Check(*static_cast<int*>(__tls_get_addr_nid_postfix(&second)) == 22);
    });
    worker.join();
    Check(*static_cast<int*>(__tls_get_addr_nid_postfix(&first)) == 33);
    bool threw = false;
    try {
        __tls_get_addr_nid_postfix(nullptr);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    Check(threw);
    return 0;
}
