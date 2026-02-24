```table-of-contents
title: 
style: nestedList # TOC style (nestedList|nestedOrderedList|inlineFirstLevel)
minLevel: 0 # Include headings from the specified level
maxLevel: 0 # Include headings up to the specified level
include: 
exclude: 
includeLinks: true # Make headings clickable
hideWhenEmpty: false # Hide TOC if no headings are found
debugInConsole: false # Print debug info in Obsidian console
```

# Revisions

|             |            |         |                                                                                                        |
| ----------- | ---------- | ------- | ------------------------------------------------------------------------------------------------------ |
| **Version** | **When**   | **Who** | **What**                                                                                               |
| 0.1         | 02/24/2025 | Biaggio | Start familiarization of documentation, plan the KidsBytes considering mainly points and restrictions. |
|             |            |         |                                                                                                        |
|             |            |         |                                                                                                        |
|             |            |         |                                                                                                        |
|             |            |         |                                                                                                        |
|             |            |         |                                                                                                        |
|             |            |         |                                                                                                        |
# Summary – Case study KidsBytes

The Beacon Locator Puzzle Box project from KidsBytes company was initiated to address heavily guided tours that rely on a lot of verbal explanations for a children audience in **Bronkhorst**, replacing them with an interactive, dynamic tour that promotes an engaging adventure of self-discovery of **Bronkhorst’s vision**. The client, represented by **Bronkhorst**, a highly technical company specialized in flow measurement and control systems, had an educational reason to showcase its building.

The main goal of the project is to realize a Puzzle Box that is a minimal but practical embodiment of a smart device capable of guiding small kids through the unknown. The planned key features include autonomous location via iBeacons, a user-friendly interface, puzzles, and a reward for finishing the tour. From the outset, the project required the integration of both hardware and software, leveraging the strengths of a diverse, cross-functional team.

The team adopted a flexible development process based on priorities. Test procedures were conducted to determine the behavior of the software and hardware, allowing design validation before committing to real-world builds. A significant challenge was the limited documentation of the FDRM MXCA153 board, which caused slow initialization in both hardware and software. Additionally, the topic of a user interface for kids is something out of scope, as it demands extreme simplicity in the logic and use of the Puzzle Box.

Key technological selections included the FDRM MXCA153 board and the HM10 Bluetooth module. Development began using the VS Code IDE to accelerate prototyping and support early software development alongside hardware iterations.

# Introduction

How do you motivate children in an environment filled with machines, complex tools, technical language, and images that are far from understandable even for adults—let alone for kids? Adding another layer in which the children audience must listen to complicated logic could become a boring nightmare for them. Therefore, the client recognizes that such struggles are unnecessary. As a consequence, a different approach must be put in place—an approach that captures the bright energy and mood of kids in order to intelligently and emotionally wrap the vision of the company **Bronkhorst**.

This project’s main goal is to deliver a stable, educational Puzzle Box from KidsBytes that maximizes the practical teaching of **Bronkhorst** to society under the specific requirements of the client, providing a discovery-based approach to the company and increasing future visits. The realization will be based on the FDRM MXCA153, with the team following an iterative prototyping process and maintaining close communication with the client, who brings a commercial vision. Work will be organized in clearly defined increments—each tested and refined in consultation—and bounded by preconditions such as fixed hardware, selected components, strict cost limits, and a twelve-week project timeframe.

The report is organized to transparently guide the reader through the development journey: 
* Chapter 2 summarizes the preliminary research and early decisions; 
* Chapter 3 details functional requirements and describes the target system’s main functions; 
* Chapter 4 addresses technical design choices and underlying architecture; 
* Chapter 5 documents the realization of both hardware and software;
* Chapter 6 presents the outcomes of prototype testing; finally, 
* Chapter 7 discusses conclusions and recommendations, supporting future reflection and potential industrial rollout.

# Context at a Glance

The Puzzle Box must guide children aged 8–10 through 5 rooms at Bronkhorst High-Tech (Ruurlo) via iBeacon detection, interactive puzzles, and an electronically controlled lock — all running on the FRDM-MCXA153 and HM-10 BLE module, with a companion PC application for admin and logging.

# Functional design

The functional design establishes a comprehensive specification for the Box puzzle system, developed in close consultation with the customer to ensure alignment with their vision for learning interactive approach This section defines **what** the Boz puzzle must accomplish without prescribing **how** these capabilities will be technically implemented.
# Functional specifications

The Puzzle Box core functionality centers on two primary operational modes: autonomous assistance guide navigation and custom configuration of navigation, allowing the client to change which path the audience can navigate within the admin configuration. Supporting these modes are essential subsystems, including motor control with variable speed, real-time display of operational status, persistent time tracking, and a user interface for mode selection. Optional features such as wireless communication and enhanced status indicators provide pathways for future system expansion while maintaining focus on the essential requirements within the current project scope.

## SMART Functional Specifications via MoSCoW

### F1 – iBeacon Reception & Location Detection

| F    | MoSCoW | Description                                                                                                                                    |
| ---- | ------ | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| F1   | M      | The PuzzleBox must continuously scan for iBeacon signals using the HM-10 CC2541 BLE module during all active sessions.                         |
| F1.1 | M      | The PuzzleBox must distinguish between at least 5 distinct iBeacon locations using UUID, Major, and Minor identifiers.                         |
| F1.2 | M      | The BLE beacon signal reception status must always be visible on the display during operation.                                                 |
| F1.3 | S      | When BLE signal is lost, the system should retain the last valid checkpoint and show a "searching" indicator to the user.                      |
| F1.4 | S      | The system should implement RSSI signal tolerance margins to compensate for BLE interference from industrial metal structures in the building. |
| F1.5 | C      | The system could attempt automatic BLE reconnection within 10 seconds after a disconnection event.                                             |
### F2 – Electronic Lock

| F    | MoSCoW | Description                                                                                                                    |
| ---- | ------ | ------------------------------------------------------------------------------------------------------------------------------ |
| F2   | M      | The PuzzleBox must electronically lock and unlock using an actuator controlled by the FRDM-MCXA153 GPIO output.                |
| F2.1 | M      | The lock must only open when the correct puzzle answer is entered and the correct beacon location is confirmed simultaneously. |
| F2.2 | M      | The lock must remain closed in all other states, including during power-on, BLE search, and admin mode.                        |
| F2.3 | S      | The lock should provide an auditory or tactile confirmation signal within 3 second of a successful unlock event.               |
| F2.4 | S      | An admin override must be available to manually unlock the box during error recovery or testing.                               |
### F3 – User Interface & Display

| #    | MoSCoW | Description                                                                                                                                         |
| ---- | ------ | --------------------------------------------------------------------------------------------------------------------------------------------------- |
| F3   | M      | The PuzzleBox must display the current beacon location and the next target destination using the LVGL graphical interface on the FRDM-MCXA153.      |
| F3.1 | M      | All on-screen text, icons, and prompts must be comprehensible to children aged 8–10 without adult assistance, using pictograms and simple language. |
| F3.2 | M      | The UI must provide a clear visual state change (color, animation, or icon) upon successful puzzle completion at a location.                        |
| F3.3 | S      | The interface should display a step progress indicator (e.g., "Room 2 of 5") so children know where they are in the tour.                           |
| F3.4 | S      | The display should show a low-signal warning icon when BLE reception is below a reliable threshold.                                                 |
| F3.5 | C      | The interface could display a simple animated reward screen (e.g., celebration animation) when the final room is completed.                         |
### F4 – Puzzle System

| F    | MoSCoW | Description                                                                                                                                                                 |
| ---- | ------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F4   | M      | The PuzzleBox must support at least one interactive on-device puzzle (e.g., number code entry, mastermind, or tic-tac-toe) operable via the touchscreen or input interface. |
| F4.1 | M      | The final destination room must require children to enter a numeric code collected across all 5 intermediate rooms to unlock the box.                                       |
| F4.2 | S      | The system should support at least one room-contextual puzzle where children answer a question about the room's machine or function.                                        |
| F4.3 | S      | An incorrect puzzle answer must provide visual feedback without resetting tour progress, allowing reattempt.                                                                |
| F4.4 | C      | The system could support a time-bound puzzle variant with a visible countdown timer.                                                                                        |
| F4.5 | W      | Different puzzle difficulty levels per target group will not be implemented in Phase 1 (Period 3 scope).                                                                    |
### F5 – Admin Configuration Mode

| F    | MoSCoW | Description                                                                                                                                         |
| ---- | ------ | --------------------------------------------------------------------------------------------------------------------------------------------------- |
| F5   | M      | The PuzzleBox must provide a password-protected admin mode accessible only to authorized staff, not reachable during normal child-facing operation. |
| F5.1 | M      | In admin mode, the administrator must be able to set, reset, and remove beacon target locations using either the box itself or the PC application.  |
| F5.2 | M      | In admin mode, the administrator must be able to set or update puzzle answers for each room checkpoint.                                             |
| F5.3 | S      | Admin mode should be accessible via a hidden menu sequence on the box (e.g., long-press combination) as a fallback when the PC is unavailable.      |
| F5.4 | C      | Admin mode could display a summary of all configured beacon locations and puzzle answers for quick verification before a tour.                      |
### F6 – PC Companion Application

| F    | MoSCoW | Description                                                                                                                                    |
| ---- | ------ | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| F6   | M      | A PC application with a graphical user interface must be developed and must establish a connection with the PuzzleBox.                         |
| F6.1 | M      | The PC application must display logging data including the last route taken, time spent at each target location, and total distance travelled. |
| F6.2 | M      | The PC application must allow the administrator to configure beacon target locations and set puzzle answers.                                   |
| F6.3 | S      | The PC application should export tour log data to a readable file format (e.g., CSV or plain text) for review by Bronkhorst staff.             |

### F7 – Power Management & Data Persistence

| F    | MoSCoW | Description                                                                                                                                                     |
| ---- | ------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F7   | M      | The PuzzleBox must be powered by a power bank housed in a separate compartment that can be accessed and replaced without opening the electronic lock.           |
| F7.1 | M      | All log data (route, timestamps, puzzle results) must be written to non-volatile memory continuously and must be fully retained after a power cycle or removal. |
| F7.2 | S      | The system should display a low-battery warning on screen when the power bank charge falls below a safe operating threshold.                                    |
| F7.3 | C      | The system could estimate and display the remaining operational time based on current battery level.                                                            |
### F8 – Physical Enclosure & Safety

| F    | MoSCoW | Description                                                                                                                                                  |
| ---- | ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| F8   | M      | The enclosure must be tamper-resistant for children aged 8–10: no exposed cables, protected screws, and no accessible internal components during normal use. |
| F8.1 | M      | The enclosure must withstand being dropped from a height of 1 meter onto a hard floor without causing hardware damage or software corruption.                |
| F8.2 | M      | The total hardware component cost, excluding the FRDM-MCXA153 board, must not exceed €50.                                                                    |
| F8.3 | S      | The enclosure should incorporate shock-absorbing internal mounting or padding to stabilize the FRDM-MCXA153 and HM-10 module under physical impact.          |
| F8.4 | C      | The exterior of the box could incorporate Bronkhorst branding or visual design elements to reinforce the educational theme for children.                     |