.. _blinky-frills:

Blinky Frills
######

Overview
********

A simple blinky demo showcasing the use of GPIO input and output with interrupts.
The demo is operated with the BUTTON1 (switch mode), BUTTON2(slower) and BUTTON4(faster).

The speed and current mode are printed to UART if it is configured.

Requirements
************

The board hardware must have a push 4 buttons connected via a GPIO pin. These are
called "User buttons" on many of Zephyr's :ref:`boards`.

The buttons must be configured using the sw0, sw1 and sw3, 
aliases, usually in the BOARD.dts file. You will
see one of these errors if you try to build this sample for an unsupported board:

.. code-block:: none

   Unsupported board: sw0 devicetree alias is not defined
   Unsupported board: sw1 devicetree alias is not defined
   Unsupported board: sw2 devicetree alias is not defined
   Unsupported board: sw4 devicetree alias is not defined

You may see additional build errors if the aliases exist, but are not
properly defined.

The demo additionally requires the ``led0``, ``led1``, ``led2`` and ``led3``
devicetree aliases.
