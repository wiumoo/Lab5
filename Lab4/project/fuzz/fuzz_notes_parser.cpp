#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>

// A tiny target that is easy for afl++ to exercise.
// We intentionally keep it simple and deterministic.
//
// Build (clang recommended):
//   afl-clang-fast++ -O0 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
//     fuzz_notes_parser.cpp -o fuzz_notes_parser
// Run:
//   mkdir in; echo "AAAA" > in/seed
//   afl-fuzz -i in -o out -- ./fuzz_notes_parser @@

static int parse_note(const uint8_t *data, size_t size) {
    // A few branching conditions to create coverage guidance.
    if (size == 0) return 0;
    if (size > 4096) return 0;

    // Accept only printable-ish bytes.
    for (size_t i = 0; i < size; i++) {
        if (data[i] == 0) return 0;
    }

    // Simple pattern triggers.
    if (size >= 4 && std::memcmp(data, "LEDG", 4) == 0) {
        return 1;
    }

    // Another pattern with length dependent branch.
    if (size >= 8 && std::memcmp(data, "BUDGET:", 7) == 0) {
        return 2;
    }

#ifdef ENABLE_CRASH_DEMO
    // Crash demo for fuzzing reports:
    // Build with -DENABLE_CRASH_DEMO and ASAN/UBSAN enabled.
    // Triggered by inputs starting with "CRASH".
    if (size >= 5 && std::memcmp(data, "CRASH", 5) == 0) {
        // BUG: assumes the fuzz input is NUL-terminated.
        // With ASAN, afl++ can quickly find a crashing input.
        char small[8];
        std::strcpy(small, reinterpret_cast<const char *>(data));
        (void)small;
    }
#endif

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) return 0;

    const char *path = argv[1];
    FILE *f = std::fopen(path, "rb");
    if (!f) return 0;

    std::fseek(f, 0, SEEK_END);
    long len = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);

    if (len <= 0 || len > 1 << 20) {
        std::fclose(f);
        return 0;
    }

    uint8_t *buf = (uint8_t *)std::malloc((size_t)len);
    if (!buf) {
        std::fclose(f);
        return 0;
    }

    size_t n = std::fread(buf, 1, (size_t)len, f);
    std::fclose(f);

    (void)parse_note(buf, n);

    std::free(buf);
    return 0;
}
