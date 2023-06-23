//! These binaries work on the following targets:
//! - DragonFlyBSD
//! - FreeBSD
//! - Minix3
//! - NetBSD
//! - OpenBSD
//! - Ubuntu GNU/Linux
const std = @import("std");
const Build = std.Build;
const assert = std.debug.assert;

// ar          Archive manager.

// elfcopy     Copy and translate between object formats.
// elfdump     Diagnostic tool.
// findtextrel Find undesired text relocations.
// libdwarf    DWARF access library.
// libelf      ELF access library.
// mcs         Manage comment sections.
// ranlib      Add archive symbol tables to an archive.
// readelf     Display ELF information.
// size        List object sizes.
// strings     Extract printable strings.
// strip       Discard information from ELF objects.
fn capture_stdout_with_basename(run: *Build.Step.Run, basename: []const u8) Build.FileSource {
    assert(run.stdio != .inherit);

    if (run.captured_stdout) |output| return .{ .generated = &output.generated_file };

    const output = run.step.owner.allocator.create(Build.Step.Run.Output) catch @panic("OOM");
    output.* = .{
        .prefix = "",
        .basename = basename,
        .generated_file = .{ .step = &run.step },
    };
    run.captured_stdout = output;
    return .{ .generated = &output.generated_file };
}

const ExeOptions = struct {
    name: []const u8,
    sources: []const []const u8,
    libraries: []const *std.Build.Step.Compile,
    target: std.zig.CrossTarget,
    optimize: std.builtin.OptimizeMode,
};

fn add_elftoolchain_exe(b: *Build, options: ExeOptions) void {
    const exe = b.addExecutable(.{
        .name = options.name,
        .target = options.target,
        .optimize = options.optimize,
        .link_libc = true,
    });
    exe.addCSourceFiles(options.sources, &.{});
    exe.addIncludePath("common");

    for (options.libraries) |lib|
        exe.linkLibrary(lib);

    b.installArtifact(exe);
}

pub fn build(b: *Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // libelf
    const elf_types_m4 = b.addSystemCommand(&.{ "m4", "libelf/elf_types.m4" });
    const elf_types = capture_stdout_with_basename(elf_types_m4, "elf_types.c");

    const libelf_convert_m4 = b.addSystemCommand(&.{ "m4", "-D", "SRCDIR=./libelf", "libelf/libelf_convert.m4" });
    const libelf_convert = capture_stdout_with_basename(libelf_convert_m4, "libelf_convert.c");

    const libelf_fsize_m4 = b.addSystemCommand(&.{ "m4", "-D", "SRCDIR=./libelf", "libelf/libelf_fsize.m4" });
    const libelf_fsize = capture_stdout_with_basename(libelf_fsize_m4, "libelf_fsize.c");

    const libelf_msize_m4 = b.addSystemCommand(&.{ "m4", "-D", "SRCDIR=./libelf", "libelf/libelf_msize.m4" });
    const libelf_msize = capture_stdout_with_basename(libelf_msize_m4, "libelf_msize.c");

    const libelf = b.addStaticLibrary(.{
        .name = "elf",
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    libelf.addCSourceFiles(libelf_srcs, &.{});
    libelf.addCSourceFileSource(.{ .source = libelf_convert, .args = &.{} });
    libelf.addCSourceFileSource(.{ .source = libelf_fsize, .args = &.{} });
    libelf.addCSourceFileSource(.{ .source = libelf_msize, .args = &.{} });
    libelf.addCSourceFileSource(.{ .source = elf_types, .args = &.{} });
    libelf.addIncludePath("common");
    libelf.addIncludePath("libelf");
    libelf.installHeader("common/sys/elfdefinitions.h", "sys/elfdefinitions.h");
    libelf.installHeader("libelf/libelf.h", "libelf.h");
    libelf.installHeader("libelf/gelf.h", "gelf.h");
    b.installArtifact(libelf);

    // libelftc
    const libelftc = b.addStaticLibrary(.{
        .name = "elftc",
        .root_source_file = .{ .path = "zig/elftc_version.zig" },
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    libelftc.addCSourceFiles(libelftc_srcs, &.{});
    libelftc.addIncludePath("common");
    libelftc.addIncludePath("libelftc");
    libelftc.linkLibrary(libelf);
    libelftc.installHeader("libelftc/libelftc.h", "libelftc.h");
    b.installArtifact(libelftc);

    // libdwarf
    const libdwarf = b.addStaticLibrary(.{
        .name = "dwarf",
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    libdwarf.addCSourceFiles(libdwarf_srcs, &.{});
    libdwarf.addIncludePath("common");
    libdwarf.addIncludePath("libdwarf");
    libdwarf.linkLibrary(libelf);
    libdwarf.installHeader("libdwarf/dwarf.h", "dwarf.h");
    libdwarf.installHeader("libdwarf/libdwarf.h", "libdwarf.h");
    b.installArtifact(libdwarf);

    // libpe
    const libpe = b.addStaticLibrary(.{
        .name = "pe",
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    libpe.addCSourceFiles(libpe_srcs, &.{});
    libpe.addIncludePath("common");
    libpe.addIncludePath("libpe");
    //libpe.linkLibrary(libelf);
    //libpe.installHeader("libpe/pe.h", "pe.h");
    //libpe.installHeader("libpe/libpe.h", "libpe.h");
    b.installArtifact(libpe);

    add_elftoolchain_exe(b, .{
        .name = "addr2line",
        .sources = &.{"addr2line/addr2line.c"},
        .libraries = &.{ libdwarf, libelftc },
        .target = target,
        .optimize = optimize,
    });

    add_elftoolchain_exe(b, .{
        .name = "brandelf",
        .sources = &.{"brandelf/brandelf.c"},
        .libraries = &.{ libelf, libelftc },
        .target = target,
        .optimize = optimize,
    });

    add_elftoolchain_exe(b, .{
        .name = "c++filt",
        .sources = &.{"cxxfilt/cxxfilt.c"},
        .libraries = &.{libelftc},
        .target = target,
        .optimize = optimize,
    });

    add_elftoolchain_exe(b, .{
        .name = "elfcopy",
        .sources = &.{
            "elfcopy/pe.c",
            "elfcopy/segments.c",
            "elfcopy/ascii.c",
            "elfcopy/binary.c",
            "elfcopy/archive.c",
            "elfcopy/main.c",
            "elfcopy/sections.c",
            "elfcopy/symbols.c",
        },
        .libraries = &.{ libelf, libpe },
        .target = target,
        .optimize = optimize,
    });

    add_elftoolchain_exe(b, .{
        .name = "nm",
        .sources = &.{"nm/nm.c"},
        .libraries = &.{ libdwarf, libelf, libelftc },
        .target = target,
        .optimize = optimize,
    });
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

const libdwarf_srcs = &.{
    "libdwarf/dwarf_cu.c",
    "libdwarf/dwarf_pro_die.c",
    "libdwarf/dwarf_errmsg.c",
    "libdwarf/dwarf_str.c",
    "libdwarf/libdwarf_ranges.c",
    "libdwarf/dwarf_init.c",
    "libdwarf/dwarf_pro_expr.c",
    "libdwarf/libdwarf_die.c",
    "libdwarf/dwarf_arange.c",
    "libdwarf/dwarf_lineno.c",
    "libdwarf/libdwarf_abbrev.c",
    "libdwarf/dwarf_pro_finish.c",
    "libdwarf/dwarf_dealloc.c",
    "libdwarf/dwarf_pro_macinfo.c",
    "libdwarf/dwarf_pro_attr.c",
    "libdwarf/dwarf_pro_reloc.c",
    "libdwarf/libdwarf_info.c",
    "libdwarf/dwarf_frame.c",
    "libdwarf/libdwarf_init.c",
    "libdwarf/libdwarf_reloc.c",
    "libdwarf/dwarf_sections.c",
    "libdwarf/libdwarf_rw.c",
    "libdwarf/libdwarf_attr.c",
    "libdwarf/dwarf_reloc.c",
    "libdwarf/dwarf_pro_init.c",
    "libdwarf/dwarf_pro_frame.c",
    "libdwarf/dwarf_pro_sections.c",
    "libdwarf/libdwarf_elf_access.c",
    "libdwarf/libdwarf_sections.c",
    "libdwarf/dwarf_pro_arange.c",
    "libdwarf/dwarf_pro_lineno.c",
    "libdwarf/libdwarf.c",
    "libdwarf/dwarf_abbrev.c",
    "libdwarf/libdwarf_frame.c",
    "libdwarf/dwarf_macinfo.c",
    "libdwarf/dwarf_attrval.c",
    "libdwarf/libdwarf_loc.c",
    "libdwarf/libdwarf_macinfo.c",
    "libdwarf/libdwarf_nametbl.c",
    "libdwarf/dwarf_die.c",
    "libdwarf/dwarf_form.c",
    "libdwarf/libdwarf_arange.c",
    "libdwarf/libdwarf_lineno.c",
    "libdwarf/libdwarf_elf_init.c",
    "libdwarf/dwarf_attr.c",
    "libdwarf/dwarf_finish.c",
    "libdwarf/libdwarf_loclist.c",
    "libdwarf/libdwarf_error.c",
    "libdwarf/dwarf_seterror.c",
    "libdwarf/dwarf_dump.c",
    "libdwarf/libdwarf_str.c",
    "libdwarf/dwarf_loclist.c",
    "libdwarf/dwarf_ranges.c",
};

const libelftc_srcs = &.{
    "libelftc/elftc_bfdtarget.c",
    "libelftc/libelftc_hash.c",
    "libelftc/libelftc_dem_gnu3.c",
    "libelftc/libelftc_dem_arm.c",
    "libelftc/libelftc_vstr.c",
    "libelftc/libelftc_bfdtarget.c",
    "libelftc/elftc_reloc_type_str.c",
    "libelftc/elftc_demangle.c",
    "libelftc/elftc_timestamp.c",
    "libelftc/elftc_string_table.c",
    "libelftc/elftc_set_timestamps.c",
    "libelftc/elftc_copyfile.c",
    "libelftc/libelftc_dem_gnu2.c",
};

const libpe_srcs = &.{
    "libpe/libpe_buffer.c",
    "libpe/libpe_coff.c",
    "libpe/libpe_dos.c",
    "libpe/libpe_init.c",
    "libpe/libpe_rich.c",
    "libpe/libpe_section.c",
    "libpe/libpe_utils.c",
    "libpe/pe_buffer.c",
    "libpe/pe_cntl.c",
    "libpe/pe_coff.c",
    "libpe/pe_dos.c",
    "libpe/pe_flag.c",
    "libpe/pe_init.c",
    "libpe/pe_rich.c",
    "libpe/pe_section.c",
    "libpe/pe_symtab.c",
    "libpe/pe_update.c",
};
