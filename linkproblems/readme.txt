rlink is not placing the init data at the correct places at the
end of the program. at least if this has not already been addressed

The version I have is from toolshed_linux_0.9.tgz, date 2005-09-17.
Since I don't have my own internet access, it's a bit inconvenient
to get up-to-date sources.  I believe I'd have to install cvs
onto someone else's system - everyone I know has Windows, and if
anything happened to their computer, the progs I installed would
surely have caused the problem.... ;)

In this tarball, a test program was build from the sources in the
directory "source"

The rof's were linked on the COCO (dir cocolnk) and the same rof's
were linked by the cross linker (dir crosslnk).

Included is a disassembled listing of each module, for comparison,
with comments in crosslink/xlnk.list showing what should have been
there (it's also in the disassembly of the "real" file
(cocolnk/coco.list).

If not already fixed, this will need to be done before a complete
package is usable.
