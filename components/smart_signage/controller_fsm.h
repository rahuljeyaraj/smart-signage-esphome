// #pragma once

// #include "message_protocol.h"
// #include <etl/fsm.h> // ETL finite-state-machine core
// #include <etl/message.h>
// #include <etl/string.h>
// #include "logger.h"

// namespace esphome::smart_signage
// {

//     // ──────────────────────────
//     // 1) Message (Event) types
//     // ──────────────────────────
//     enum : uint16_t
//     {
//         MSG_PRESENCE_DETECTED = 1,
//         MSG_FALL_DETECTED
//     };

//     struct PresenceDetectedMsg : etl::message<MSG_PRESENCE_DETECTED>
//     {
//     };
//     struct FallDetectedMsg : etl::message<MSG_FALL_DETECTED>
//     {
//     };

//     // Forward-declare the state list
//     struct IdleState;
//     struct ActiveState;
//     struct AlertState;

//     // ──────────────────────────
//     // 2) FSM class
//     // ──────────────────────────
//     class ControllerFsm : public etl::fsm
//     {
//     public:
//         ControllerFsm(QueueHandle_t led_q, QueueHandle_t spk_q)
//             : led_queue_(led_q), speaker_queue_(spk_q) {}

//         // Queues are public so states can access them
//         QueueHandle_t led_queue_;
//         QueueHandle_t speaker_queue_;
//     };

//     // ──────────────────────────
//     // 3) State definitions
//     // ──────────────────────────
//     struct IdleState : etl::fsm_state<ControllerFsm, IdleState>
//     {
//         void on_receive(ControllerFsm &fsm, const PresenceDetectedMsg &)
//         {
//             Command cmd{CmdId::LedOn, static_cast<uint8_t>(80)};
//             xQueueSend(fsm.led_queue_, &cmd, 0);
//             this->set_state<ActiveState>(fsm); // transition
//             LOGI("ss", "Idle→Active (presence)");
//         }
//     };

//     struct ActiveState : etl::fsm_state<ControllerFsm, ActiveState>
//     {
//         void on_receive(ControllerFsm &fsm, const FallDetectedMsg &)
//         {
//             etl::string<64> path;
//             path.assign("audio/alert1.mp3");
//             Command cmd{CmdId::PlayAudio, path};
//             xQueueSend(fsm.speaker_queue_, &cmd, 0);
//             this->set_state<AlertState>(fsm); // transition
//             LOGI("ss", "Active→Alert (fall)");
//         }
//     };

//     struct AlertState : etl::fsm_state<ControllerFsm, AlertState>
//     {
//         void on_receive(ControllerFsm &fsm, const PresenceDetectedMsg &)
//         {
//             Command cmd{CmdId::LedOn, static_cast<uint8_t>(80)};
//             xQueueSend(fsm.led_queue_, &cmd, 0);
//             this->set_state<ActiveState>(fsm); // transition
//             LOGI("ss", "Alert→Active (presence)");
//         }
//     };

// } // namespace esphome::smart_signage
