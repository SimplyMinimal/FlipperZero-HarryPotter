#include <assets_icons.h>
#include <furi.h>
#include <gui/elements.h>
#include <gui/gui.h>
#include <gui/icon.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {
    EventTypeTick,
    EventTypeInput,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} HarryPotterEvent;

static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    canvas_clear(canvas);

    // Top Banner
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 32, 10, "Avada Kedavra!");
    canvas_draw_icon(canvas, 21, 2, &I_badusb_10px);
    canvas_draw_icon(canvas, 96, 2, &I_badusb_10px);

    // Harry
    canvas_draw_icon(canvas, 46, 34, &I_FaceNormal_29x14);
    canvas_draw_icon(canvas, 54, 19, &I_Voltage_16x16);

    // Wand
    canvas_draw_line(canvas, 79, 47, 97, 35); //Left side of Wand
    canvas_draw_line(canvas, 96, 36, 77, 47); //Right side of Wand
    canvas_draw_line(canvas, 80, 47, 78, 47); // Bottom of Wand
    canvas_draw_icon(canvas, 99, 29, &I_Pin_star_7x7); //Star on Wand

    // Voldemort
    canvas_draw_icon(canvas, 97, 46, &I_FaceConfused_29x14);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    //Checking if the context is not null
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;

    HarryPotterEvent event = {.type = EventTypeInput, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void timer_callback(FuriMessageQueue* event_queue) {
    //Checking if the context is not null
    furi_assert(event_queue);

    HarryPotterEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

int32_t harry_potter_app(void* p) {
    UNUSED(p);

    //The current HarryPotterEvent custom type event
    HarryPotterEvent event;
    //Event queue with 8 elements of size HarryPotterEvent
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(HarryPotterEvent));

    //Create a new viewport
    ViewPort* view_port = view_port_alloc();
    //Create a render callback, without context
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    //Create a callback for keystrokes, pass as context
    //our message queue to push these events into it
    view_port_input_callback_set(view_port, input_callback, event_queue);

    //Creating a GUI Application
    Gui* gui = furi_record_open(RECORD_GUI);
    //Connect view port to GUI in full screen mode
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    //We create a periodic timer with a callback, where as
    //context will be passed our event queue
    FuriTimer* timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, event_queue);
    //Starting the timer
    furi_timer_start(timer, 500);

    //Turn on notifications
    NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);

    //Infinite event queue processing loop
    while(1) {
        //Select an event from the queue into the event variable (wait indefinitely if the queue is empty)
        //and check that we managed to do it
        furi_check(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk);

        //Our event is a button click
        if(event.type == EventTypeInput) {
            //If the "back" button is pressed, then we exit the loop, and hence the application
            if(event.input.key == InputKeyBack) {
                break;
            }
            //Our event is a triggered timer
        } else if(event.type == EventTypeTick) {
            //We send notification of blinking blue LED
            notification_message(notifications, &sequence_blink_blue_100);
        }
    }

    //Clearing the timer
    furi_timer_free(timer);

    //Special cleaning of memory occupied by the queue
    furi_message_queue_free(event_queue);

    //We clean the created objects associated with the interface
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    //Clearing notifications
    furi_record_close(RECORD_NOTIFICATION);

    return 0;
}
