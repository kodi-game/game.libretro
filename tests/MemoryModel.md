# Internal memory model and upstream audit

Audited on 2026-09-19 against:

- [libretro-common `38907d07`, `include/libretro.h`](https://github.com/libretro/libretro-common/blob/38907d073ff3be7b89b87a9948bc31ec418d1222/include/libretro.h):
  `RETRO_ENVIRONMENT_SET_MEMORY_MAPS`, `retro_memory_descriptor`, the SNES example,
  and `retro_init` / `retro_unload_game` / `retro_deinit`.
- [RetroArch `10896259`, `runloop.c`](https://github.com/libretro/RetroArch/blob/108962597484c3263b7eb8a3d7e6b3fa9341b0dc/runloop.c):
  `mmap_preprocess_descriptors` and its four bit helpers, the environment callback,
  `core_unload_game`, `runloop_event_deinit_core`, `uninit_libretro_symbols`, and
  `runloop_system_info_free`.
- [RetroArch achievement memory](https://github.com/libretro/RetroArch/blob/108962597484c3263b7eb8a3d7e6b3fa9341b0dc/cheevos/cheevos.c):
  `rcheevos_init_memory` / `rcheevos_refresh_memory`; also the cheat refresh in
  `runloop.c` and `cheat_manager_initialize_memory` in `cheat_manager.c`.
- [Disconnect fix `91bf7b55`](https://github.com/libretro/RetroArch/commit/91bf7b553acedaa9f8b31b204c18448c0a6ff823)
  and [runtime replacement `18f79a8a`](https://github.com/libretro/RetroArch/commit/18f79a8a8df6fc5925a8305a437b6446fabd1402).
- rcheevos [v12.3.0](https://github.com/RetroAchievements/rcheevos/blob/v12.3.0/src/rc_libretro.c)
  and current [`14331732`](https://github.com/RetroAchievements/rcheevos/blob/1433173220a7eaede6a9ed7a18e94117be1821e0/src/rc_libretro.c),
  including `src/rc_libretro.h` and
  [`test/test_rc_libretro.c`](https://github.com/RetroAchievements/rcheevos/blob/1433173220a7eaede6a9ed7a18e94117be1821e0/test/test_rc_libretro.c).

## Ownership and replacement

The memory components live in `src/memory`. `CGameLibRetro` owns
`CLibretroMemory`, which owns a `CMemoryMap` value and the
loaded core's two standard-memory function pointers. The DLL outlives this
component. `MemoryDescriptor` owns all normalized fields and its address-space
string; its `data` pointer only borrows core RAM. No RAM is copied or dereferenced
by normalization. Null and empty address-space names are equivalent.

`SET_MEMORY_MAPS` copies and normalizes a candidate before swapping it into the
current value. Null descriptor arrays with nonzero counts, invalid normalization,
allocation failure, and unrepresentable range arithmetic return false without
changing either the current map or generation. Zero descriptors explicitly clear
the current map and enable standard-memory fallback. A nonempty map containing
only inaccessible/informational descriptors is still a map, not a request for
fallback. Address-space names, flags and descriptor ordering are preserved;
additional speculative format restrictions are not imposed.

Normalization follows current RetroArch, including `highest_reachable` and
preservation of disconnect bits on both sides of the buffer length. Safety
checks additionally reject overflow in an implicit range's `start + len - 1`,
an inferred length of `SIZE_MAX + 1`, and a nonnull buffer's `offset + len - 1`.
Explicit-select bank switching and non-power-of-two lengths remain legal.
Bit operations use unsigned `size_t`, and no shift is as wide as the type.

The environment singleton only dispatches to the owning addon. It neither owns
nor exports a global memory map, and drops its addon/core/bridge pointers on
teardown. `CGameLibRetro::GetMemory` and RA fallback both use `CLibretroMemory`.
Flat calls are made only during a content attempt/session with installed core
functions. Outputs are cleared before lookup; an unavailable buffer returns
false internally while the existing Kodi `GAME_ERROR_NO_ERROR` result is preserved.

## Retained-core lifetime decision

The specification requires copying descriptor metadata and describes backing
pointer validity for the session. It does **not** explicitly guarantee that a
content buffer survives `retro_unload_game`, nor that a superseded map's pointers
can be reused. The runtime-change commit removed the restriction to init/load
callbacks. RetroArch replaces its map and refreshes achievements and active
cheat memory on every such call. Its ordinary teardown unloads content, calls
`retro_deinit`, unloads symbols, and frees the system's descriptors. It therefore
does not demonstrate a policy for multiple contents under one `retro_init`.

For this wrapper's retained core, the following is an explicit lifetime policy
inferred from publication phase, rather than a stronger upstream guarantee:

| Event | Current map | Memory generation |
| --- | --- | --- |
| Construct | Empty; no installed core functions | 0 |
| Initialize memory before setting environment / `retro_init` | Empty; install core functions | Advance |
| Map published before the first content attempt | Core initialization map | Advance on acceptance |
| Begin a load or standalone attempt | Preserve current init map | Advance, including flat-memory views |
| Valid map published during load/run | Replace entire map; content lifetime | Advance, even for identical metadata |
| Rejected map | Preserve map **and its existing lifetime classification** | Unchanged |
| Failed load or content unload | Discard current content map; preserve an unreplaced init map | Advance |
| Next retained load without a new map | Use unreplaced init map, or no map and flat fallback | Advance |
| Core deinitialization | Clear all maps and installed core functions | Advance |

An initialization map that is still current remains a core-session map across
content loads. A content replacement permanently supersedes it: we deliberately
keep no hidden baseline to restore. The core must publish a fresh map if it
wants to reintroduce those mappings. This avoids retaining content pointers or
resurrecting pointers invalidated by an earlier replacement. A rejected
replacement never changes the original map's lifetime.

`CCheevos::Deinitialize` drains queued client callbacks while memory is valid,
destroys the client, and releases its `CCheevosMemory` before `retro_unload_game`
or `retro_deinit`. The wrapper destructor also unloads content if the caller did
not explicitly do so. Failed load cleanup runs before a path fallback or later
retry; it never calls `retro_unload_game` for a failed attempt. Deinit is guarded
so failed addon creation does not call an uninitialized core. Stream handling is
otherwise preserved.

Memory generation versions all borrowed memory views associated with the current
core/content state: descriptor topology and backing pointers, flat
`retro_get_memory_*` buffers, and content lifetime invalidation. The existing
`Generation()` name is kept because its owning `CLibretroMemory` already supplies
that context. `Initialize`, `BeginContent`, `EndContent`, `Deinitialize`, and
each accepted `SetMemoryMap` advance the same counter; rejected maps do not.

It is a `uint64_t` counter local to the owning addon instance, not a hash of
strings/pointers or a public ABI field. Reinitialization does not reset it.
Zero is not a validity sentinel, and wraparound is not expected within an addon
lifetime. Consumers must discard cached views from an older generation;
comparisons are meaningful only while the same owner is alive. All memory
operations and rcheevos calls stay on the game thread; HTTP workers never access
them.

## RetroAchievements

`CCheevos` owns a `CCheevosMemory` for its active session. This helper references
the addon memory component, owns only `rc_libretro_memory_regions_t`, and checks
both memory generation and console ID before each nonempty read. A changed key
destroys the old regions, exports temporary C descriptors from the normalized
value, and calls the actual `rc_libretro_memory_init`. An explicit initialization
state distinguishes three cases:

- `READY` caches successful regions for the memory generation and console ID.
- `UNAVAILABLE` caches failed descriptor-backed initialization for that same key.
  Even an informational map with no accessible RAM is a descriptor map. Its
  pointers and topology cannot change without replacement, so repeated reads
  return zero without rebuilding, repeating diagnostics, or trying flat fallback.
- `UNINITIALIZED` covers both a fresh/reset view and failed no-map initialization.
  No descriptors means a null map argument and SYSTEM_RAM/SAVE_RAM fallback.
  These standard buffers can become available without a new memory generation,
  so failure stays retryable at the same key until initialization succeeds.

Every failed initialization clears partial/null regions. A changed memory
generation, changed console ID, or `Reset()` invalidates cached unavailability.
An accepted empty map advances memory generation and restores future reads to
standard-memory fallback. State, rather than a special generation value,
determines whether a cached key is meaningful. Console-specific address
translation stays entirely in rcheevos.

Both audited rcheevos revisions consume the descriptor array synchronously:
`rc_libretro_memory_get_descriptor` is used during initialization, and only the
resulting RAM pointers and sizes are copied into the regions object. Neither the
map, its strings, nor its fallback callback is retained. The temporary descriptor
array can therefore refer to the stable owned strings for that call. A scoped
`thread_local` bridge supplies the no-userdata fallback callback; RAII restores
the previous bridge (normally null), including nested initialization and unwinding.

The memory API and behavior in the two audited rcheevos revisions are otherwise
unchanged: the only memory-code diff is an explicit `uint32_t` cast of the
`disconnect_size` expression. `memory_init`, `memory_destroy`, `memory_read`,
`memory_find` / `find_avail`, descriptor selection/reduction, and standard-memory
fallback have the same behavior. Other diffs concern disallowed core settings and
string comparisons, outside the memory-model work. That work required no
dependency bump or backport; the dependency was subsequently updated to v12.5.0.

## Deterministic test provenance

`TestMemoryMap.cpp` uses constants recorded by compiling the four helpers and
`mmap_preprocess_descriptors` directly from the audited RetroArch `runloop.c` in a
temporary standalone oracle. The oracle is not a duplicate implementation in the
test suite, and running the tests requires neither RetroArch nor network access.
The complete 14-descriptor SNES example is from the audited `libretro.h`.

Representative oracle outputs (`select`, `disconnect`, `len`, in hexadecimal):

| Input | Output |
| --- | --- |
| `select=8000, disconnect=110, len=100` | `8000, 7d10, 100` |
| `select=80, disconnect=12, len=10` | `80, 52, 10` |
| `select=8000, disconnect=210, len=180` | `8000, 7a10, 180` |
| `select=c000, disconnect=2000, len=0` | `c000, 2000, 2000` |
| SNES WRAM at `7e0000` | `fe0000, 0, 20000` |
| SNES LoROM at `008000` | `408000, b08000, 80000` |
| SNES HiROM at `400000` | `400000, 800000, 400000` |
| Informational SNES address-space limit | `ffffff, 0, 1` |

RA tests use the actual pinned library, with console layouts/read boundaries
based on upstream's memory tests. They cover fallback, normalized mirrors,
buffer offsets, memory generation/console refresh, caching, rejected replacements,
changed pointers/topology, deterministic descriptor failure caching, retryable
flat-memory failure, reset/deinit, and nested/repeated callback scopes. A replaced
buffer is freed before the next RA read to make stale reads detectable under ASan.

The retained wrapper scenarios cover environment return values at init/load/run,
flat ABI behavior, failed-load retry, standalone reload without republishing,
destruction with loaded content, and a second addon using the same DLL/singleton.
Descriptor and generation assertions stay in the pure tests: they are not
observable through today's Kodi Game API, and no test-only public ABI is added.
