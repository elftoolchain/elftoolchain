//! These binaries work on the following targets:
//! - DragonFlyBSD
//! - FreeBSD
//! - Minix3
//! - NetBSD
//! - OpenBSD
//! - Ubuntu GNU/Linux
const std = @import("std");
const Build = std.Build;

// ar          Archive manager.
// addr2line   Debug tool.
// brandelf    Manage the ELF brand on executables.
// c++filt     Translate encoded symbols.
// elfcopy     Copy and translate between object formats.
// elfdump     Diagnostic tool.
// findtextrel Find undesired text relocations.
// libdwarf    DWARF access library.
// libelf      ELF access library.
// mcs         Manage comment sections.
// nm          List symbols in an ELF object.
// ranlib      Add archive symbol tables to an archive.
// readelf     Display ELF information.
// size        List object sizes.
// strings     Extract printable strings.
// strip       Discard information from ELF objects.

pub fn build(b: *Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const libelf = b.addStaticLibrary(.{
        .name = "elf",
        .target = target,
        .optimize = optimize,
    });
    libelf.addCSourceFiles(libelf_srcs, &.{});
    libelf.addIncludePath("common");
    libelf.addIncludePath("libelf");
    libelf.installHeader("libelf/libelf.h", "libelf.h");
    b.installArtifact(libelf);
}

const libelf_srcs = &.{
    "libelf/elf.c",
    "libelf/elf_begin.c",
    "libelf/elf_cntl.c",
    "libelf/elf_data.c",
    "libelf/elf_end.c",
    "libelf/elf_errmsg.c",
    "libelf/elf_errno.c",
    "libelf/elf_fill.c",
    "libelf/elf_flag.c",
    "libelf/elf_getarhdr.c",
    "libelf/elf_getarsym.c",
    "libelf/elf_getbase.c",
    "libelf/elf_getident.c",
    "libelf/elf_getversion.c",
    "libelf/elf_hash.c",
    "libelf/elf_kind.c",
    "libelf/elf_memory.c",
    "libelf/elf_next.c",
    "libelf/elf_open.c",
    "libelf/elf_phnum.c",
    "libelf/elf_rand.c",
    "libelf/elf_rawfile.c",
    "libelf/elf_scn.c",
    "libelf/elf_shnum.c",
    "libelf/elf_shstrndx.c",
    "libelf/elf_strptr.c",
    "libelf/elf_update.c",
    "libelf/elf_version.c",
    "libelf/gelf_cap.c",
    "libelf/gelf_checksum.c",
    "libelf/gelf_dyn.c",
    "libelf/gelf_ehdr.c",
    "libelf/gelf_fsize.c",
    "libelf/gelf_getclass.c",
    "libelf/gelf_move.c",
    "libelf/gelf_phdr.c",
    "libelf/gelf_rel.c",
    "libelf/gelf_rela.c",
    "libelf/gelf_shdr.c",
    "libelf/gelf_sym.c",
    "libelf/gelf_syminfo.c",
    "libelf/gelf_symshndx.c",
    "libelf/gelf_xlate.c",
    "libelf/libelf_align.c",
    "libelf/libelf_allocate.c",
    "libelf/libelf_ar.c",
    "libelf/libelf_ar_util.c",
    "libelf/libelf_checksum.c",
    "libelf/libelf_data.c",
    "libelf/libelf_ehdr.c",
    "libelf/libelf_elfmachine.c",
    "libelf/libelf_extended.c",
    "libelf/libelf_memory.c",
    "libelf/libelf_open.c",
    "libelf/libelf_phdr.c",
    "libelf/libelf_shdr.c",
    "libelf/libelf_xlate.c",
};
