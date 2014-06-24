#ifndef MACROS_H
#define MACROS_H

// Data Defines
#define MARKER_COUNT 6
#define TRACE_COUNT 6
#define PRESET_COUNT 6

#define DOCK_WIDTH 300 // Width of whole dock panel

#define ENTRY_WIDTH 290 // Width of panel entry
#define ENTRY_OFFSET 5 // panel entry initial offset
#define LABEL_W 75 // panel entry label width
#define ENTRY_H 25 // panel entry height

// Button alignments
#define BUTTON_W 130
#define BUTTON_H 25
#define BUTTON_R1_X 10
#define BUTTON_R2_X 140

#define BB_PI (3.14159265358979323)
#define BB_TWO_PI (2 * BB_PI)
#define BB_FOUR_PI (4 * BB_PI)

#define PHASE_TO_FREQ (80.0e6 / BB_TWO_PI)

// Absolute maximum sweep length regardless of decimation rate
// This creates a shorter maximum sweep time when the decimation
//  is lower. This number should be reasonable as the entirety of
//  the sweep will be plotted.
const int MAX_IQ_SWEEP_LEN = 16384;

#define DISALLOW_COPY_AND_ASSIGN(class_name) \
    class_name(class_name&); \
    class_name& operator=(class_name&);

#define DISALLOW_COPY(class_name) \
    class_name& operator=(class_name&);

#endif // MACROS_H
