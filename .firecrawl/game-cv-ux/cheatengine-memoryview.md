|     |     |
| --- | --- |
| **Memory view (Disassembly View)** |  |

* * *

**Overview**

The memory view can be reached by clicking on the memory view in the main cheat engine window.

The memory view contains several tools and functions for advanced game hackers like the autoassembler
engine, a codecave scanner, a thread generator, a memory allocator, a code dissector, a DLL injector,
debugger options, a disassembler, an assembler, a hex viewer and more.

**Manipulation**

To change assembler code just stand on the address you want to edit and type in your now code.

To edit the value of some bytes directly in memory just click on a byte and start typing in the new values.

**Hotkeys**

There are also some hotkeys, like CTRL+G to jump which will pop-up a box where you can fill in the new
address you want to go to and ctrl+f in the hex view to do a simple scan

If you have a jmp or call selected you can press SPACE to follow that address. BACKSPACE will go back.

Also, pressing CTRL+SPACE when either the disassembler is focused, or the hexview is focused, will set
the other object's location to the same as the selected one.

**Layout**

You can access the various menus along the top of the window, you can see a registers and flags pane to
the right of the screen when a breakpoint has been hit (which can be modified while debugging; also note
the arrow-button allowing access to FPU information), and you can make use of the hex-view pane at the
bottom of the window.

You can perform binary copies/pastes, force regions to be writeable, make general edits and search memory
in this view by right-clicking and choosing the corresponding option from the menu that appears. You can
also change the display type to different settings to make it that much more legible. Along the top edge of
this pane you will see module information, such as AllocBase, Size and protection

Disassemblerview:

Right-clicking on a code in the disassembler view will bring up the following options:

·**Go to address**: Go to the given address.

·**Replace with code that does nothing:** It will replace the selected codes with NOPs, so the code(s) will
do nothing. The replaced codes will be added to the code list automatically, so you can track your
changes easily.

·**Add to the code list**: Add the code to the code list.

·**Copy to clipboard**: Copy the selected area to the clipboard.

·**Change register at this location**: You can change the value of the registers whenever this code is
executed. This feature is using the debugger.

·**Toggle breakpoint**: Set/delete a breakpoint on the code.

·**Break and trace instructions**: Set a breakpoint and trace the instructions when this code is executed.
You can set a trace count and also set a condition to stop the tracing.

·**Find out what addresses this instruction accesses**: Use the debugger to find out what addresses
are accessed by the code.

·**Set/Change break condition**: You can set/change the break condition on existing breakpoints.

·**Select current function**: This option will select the current function and jump to the start of the function
in the disassembler view.

Hexview

Right-clicking on a value in the hex view will bring up the following options:

·**Edit**: Edit the value on the address.

·**Go to address**: Go to the specified address.

·**Search memory**: Search in the memory for text or an array of bytes.

·**Copy to clipboard, Paste from clipboard**: Copy and paste using the clipboard.

·**Display type**: Select a display type for the values.

·**Separators**: Set the number and range of separators.

·**Lock current rowsize**: Locking the rowsize will fix the size even if the window is resized.

·**Link with other hexview**: Link the window with another hexview so You can navigate in the windows
together, instead of controlling them separately.

·**Unlink from other hexview**: Remove the link with the other hexview.

·**Show difference**: Show the difference between the choosen memory viewer.

·**Add this address to the list**: Add the address to the cheat list.

·**Make page writable**: Give permission to write in a readable area.

·**Data breakpoint**: Set a data breakpoint on the address.

·**Data breakpoint->Break on Write**: Break when the address is written by a code.

·**Data breakpoint->Break on Access**: Break when the address is accessed by a code.

·**Data breakpoint->Break and trace**: Set a write or access breakpoint and trace the instructions when
this address is written or accessed. You can set a trace count and also set a condition to stop the
tracing.

·**Find out what accesses this address**: Use the debugger to show codes that are accessing to the
address.

·**Find out what writes to this address**: Use the debugger to show codes that are writing to the address.