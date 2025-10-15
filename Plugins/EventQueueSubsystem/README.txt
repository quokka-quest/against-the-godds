Event Queue Subsystem Plugin for Unreal Engine
==============================================

An Unreal Engine plugin that enables you to queue, process, and manage asynchronous events independently across multiple named queues. Built with full Blueprint support and real-time event control in mind.

-----------------------------
Features

• Multiple named queues (identified by `FName` tags)
• Events with prioritisation and custom payloads (via `FInstancedStruct`)
• Asynchronous event design – events must be marked complete manually
• Latent Blueprint support via EnqueueEventAndWait node (optional)
• Per-queue and global pause/resume
• Event cancellation and full queue flushing
• Max queue length support with optional truncation
• Blueprint events for queue status updates
• Blueprint pure access to queue state and tag listing

-----------------------------
How It Works

1. Use `EnqueueEvent(...)` to push an event onto a queue.
2. Queues process one event at a time.
3. Once an event is running, it must be manually marked complete using `MarkEventAsComplete(...)`.
4. The queue triggers the next event when the previous one is marked as complete.

Each queue runs independently so prioritisation is only within each queue.

-----------------------------
Blueprint Functions

• EnqueueEvent(Event, Payload, QueueTag, EventTag, Priority, Quantity)- Adds an event to the queue with the corresponding tag, setting the events tag and priority as well.
• EnqueueEventAndWait(Event, Payload, QueueTag, EventTag, Priority, Quantity)- Adds an event to the queue with the corresponding tag. Has an output pin that fires when the enqueued event has been completed.
• MarkEventAsComplete(QueueTag)- Marks the currently processing event as complete in the queue with the corresponding tag. This will automatically fire the next event.
• ToggleQueuePauseState(QueueTag, Pause)- Used to change the pause state of the queue with the corresponding tag.
• CreateQueue(QueueTag, StartPaused)- Creates a queue with the corresponding tag. Optional Boolean to start the queue in a paused state.
• EmptyQueue(QueueTag)- Flushes the queue with the corresponding tag.
• DequeueEvent(QueueTag, EventTag, RemoveAllMatching)- Removes the first event with the matching event tag from the queue with the corresponding queue tag. Optional Boolean to remove all events in that queue with matching tags.
• SetQueueMaxLength(QueueTag, MaxLength, TruncateQueue)- Sets the maximum number of events that can be queued in the queue with the corresponding tag
• ChangeQueueSystemPauseState(Paused)- Pauses/Unpauses every queue in the subsystem. Newly made queues will also be paused while global pause it true.
• FlushAllQueues()- Empties every queue.
• GetQueueState(QueueTag) – returns queue length, max length, paused state, processing state
• GetAllQueueTags() – returns all existing queue's tags
• SetQueueProcessDelay(QueueTag, delay)- Sets a delay within the queue itself between firing events

-----------------------------
Delegates

BlueprintAssignable:
• OnEventQueued(FName QueueTag)- Fires every time an event is queued, returning the tag of the queue that event was added to.
• OnQueueFinished(FName QueueTag)- Fires every time a queue finishes its last event, returning the tag of the queue that is now empty.

C++ only:
• OnEventCompleted(FGuid CompletedEventID)- Used for the EnqueueEventAndWait function to work.

-----------------------------
Data Types

FQueuedEvent: (dynamic delegate)
• A Blueprint delegate with a single `FInstancedStruct` parameter.

FQueuedEventData: (struct, not blueprintable)
• Holds the event, priority, sequence number, tag, payload, and a unique ID.

FNamedQueue: (struct, not blueprintable)
• Stores the internal queue, paused and processing state, max length, current event ID and delay between processes.

FQueueState: (blueprintable struct returned by the GetQueueState function)
• length – current event count
• maxLength – max events allowed (0 = unlimited)
• isPaused – whether this queue is paused
• isProcessing – whether an event is currently running

-----------------------------
Latent Blueprint Support

Use the `EnqueueEventAndWait` node to enqueue an event and pause Blueprint execution until it's marked as complete.

-----------------------------
Setup

1. Add the plugin to your project's Plugins folder.
2. Enable the plugin in the Unreal Editor.
3. Use the 'Get EventQueueSystem' node to access the subsystem.
   (Note: 'EnqueueEvent', 'EnqueueEventAndWait' and 'ToggleQueuePauseState' are static and do not require a target reference)

-----------------------------
Notes

• Event completion is **manual**. Be sure to call `MarkEventAsComplete` once your logic is done.
• Set max length using `SetQueueMaxLength` to prevent queues from growing indefinitely.
• Use `ChangeQueueSystemPauseState(true)` to globally pause all queues.
• You can cancel or dequeue events by their `EventTag`.
• If the quantity is passed in a 0 or less then it will be taken as 1
• Priority values can be negative.
• When settings a quantity with the EnqueueEventAndWait node the OnComplete pin will fire once all of the events have been completed.

-----------------------------
Use Cases

• Turn-based systems with queued AI logic
• Cutscene or dialogue sequence systems
• Gameplay scripting where timing or sequence order matters
• Modular async systems that require queued logic (loading, spawning, etc.)