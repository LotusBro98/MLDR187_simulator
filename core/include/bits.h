#ifndef MLDR187_SIMULATOR_BITS_H
#define MLDR187_SIMULATOR_BITS_H

#define CSR_DCSR                            0x7b0

#define CSR_DCSR_DEBUGVER_OFFSET            28
#define CSR_DCSR_DEBUGVER_LENGTH            4
#define CSR_DCSR_DEBUGVER                   (0xfU << CSR_DCSR_DEBUGVER_OFFSET)
/*
 * 0: {\tt ebreak} instructions in VS-mode behave as described in the
 * Privileged Spec.
 *
 * 1: {\tt ebreak} instructions in VS-mode enter Debug Mode.
 *
 * This bit is hardwired to 0 if the hart does not support virtualization mode.
 */
#define CSR_DCSR_EBREAKVS_OFFSET            17
#define CSR_DCSR_EBREAKVS_LENGTH            1
#define CSR_DCSR_EBREAKVS                   (0x1U << CSR_DCSR_EBREAKVS_OFFSET)
/*
 * 0: {\tt ebreak} instructions in VU-mode behave as described in the
 * Privileged Spec.
 *
 * 1: {\tt ebreak} instructions in VU-mode enter Debug Mode.
 *
 * This bit is hardwired to 0 if the hart does not support virtualization mode.
 */
#define CSR_DCSR_EBREAKVU_OFFSET            16
#define CSR_DCSR_EBREAKVU_LENGTH            1
#define CSR_DCSR_EBREAKVU                   (0x1U << CSR_DCSR_EBREAKVU_OFFSET)
/*
 * 0: {\tt ebreak} instructions in M-mode behave as described in the
 * Privileged Spec.
 *
 * 1: {\tt ebreak} instructions in M-mode enter Debug Mode.
 */
#define CSR_DCSR_EBREAKM_OFFSET             15
#define CSR_DCSR_EBREAKM_LENGTH             1
#define CSR_DCSR_EBREAKM                    (0x1U << CSR_DCSR_EBREAKM_OFFSET)
/*
 * 0: {\tt ebreak} instructions in S-mode behave as described in the
 * Privileged Spec.
 *
 * 1: {\tt ebreak} instructions in S-mode enter Debug Mode.
 *
 * This bit is hardwired to 0 if the hart does not support S-mode.
 */
#define CSR_DCSR_EBREAKS_OFFSET             13
#define CSR_DCSR_EBREAKS_LENGTH             1
#define CSR_DCSR_EBREAKS                    (0x1U << CSR_DCSR_EBREAKS_OFFSET)
/*
 * 0: {\tt ebreak} instructions in U-mode behave as described in the
 * Privileged Spec.
 *
 * 1: {\tt ebreak} instructions in U-mode enter Debug Mode.
 *
 * This bit is hardwired to 0 if the hart does not support U-mode.
 */
#define CSR_DCSR_EBREAKU_OFFSET             12
#define CSR_DCSR_EBREAKU_LENGTH             1
#define CSR_DCSR_EBREAKU                    (0x1U << CSR_DCSR_EBREAKU_OFFSET)
/*
 * 0: Interrupts (including NMI) are disabled during single stepping.
 *
 * 1: Interrupts (including NMI) are enabled during single stepping.
 *
 * Implementations may hard wire this bit to 0.
 * In that case interrupt behavior can be emulated by the debugger.
 *
 * The debugger must not change the value of this bit while the hart
 * is running.
 */
#define CSR_DCSR_STEPIE_OFFSET              11
#define CSR_DCSR_STEPIE_LENGTH              1
#define CSR_DCSR_STEPIE                     (0x1U << CSR_DCSR_STEPIE_OFFSET)
/*
 * 0: Increment counters as usual.
 *
 * 1: Don't increment any hart-local counters while in Debug Mode or
 * on {\tt ebreak} instructions that cause entry into Debug Mode.
 * These counters include the {\tt instret} CSR. On single-hart cores
 * {\tt cycle} should be stopped, but on multi-hart cores it must keep
 * incrementing.
 *
 * An implementation may hardwire this bit to 0 or 1.
 */
#define CSR_DCSR_STOPCOUNT_OFFSET           10
#define CSR_DCSR_STOPCOUNT_LENGTH           1
#define CSR_DCSR_STOPCOUNT                  (0x1U << CSR_DCSR_STOPCOUNT_OFFSET)
/*
 * 0: Increment \Rtime as usual.
 *
 * 1: Don't increment \Rtime while in Debug Mode.  If all harts
 * have \FcsrDcsrStoptime=1 and are in Debug Mode then \Rmtime
 * is also allowed to stop incrementing.
 *
 * An implementation may hardwire this bit to 0 or 1.
 */
#define CSR_DCSR_STOPTIME_OFFSET            9
#define CSR_DCSR_STOPTIME_LENGTH            1
#define CSR_DCSR_STOPTIME                   (0x1U << CSR_DCSR_STOPTIME_OFFSET)
/*
 * Explains why Debug Mode was entered.
 *
 * When there are multiple reasons to enter Debug Mode in a single
 * cycle, hardware should set \FcsrDcsrCause to the cause with the highest
 * priority.
 *
 * 1: An {\tt ebreak} instruction was executed. (priority 3)
 *
 * 2: A Trigger Module trigger fired with action=1. (priority 4)
 *
 * 3: The debugger requested entry to Debug Mode using \FdmDmcontrolHaltreq.
 * (priority 1)
 *
 * 4: The hart single stepped because \FcsrDcsrStep was set. (priority 0, lowest)
 *
 * 5: The hart halted directly out of reset due to \Fresethaltreq. It
 * is also acceptable to report 3 when this happens. (priority 2)
 *
 * 6: The hart halted because it's part of a halt group. (priority 5,
 * highest) Harts may report 3 for this cause instead.
 *
 * Other values are reserved for future use.
 */
#define CSR_DCSR_CAUSE_OFFSET               6
#define CSR_DCSR_CAUSE_LENGTH               3
#define CSR_DCSR_CAUSE                      (0x7U << CSR_DCSR_CAUSE_OFFSET)
/*
 * Extends the prv field with the virtualization mode the hart was operating
 * in when Debug Mode was entered. The encoding is described in Table
 * \ref{tab:privmode}.
 * A debugger can change this value to change the hart's virtualization mode
 * when exiting Debug Mode.
 * This bit is hardwired to 0 on harts that do not support virtualization mode.
 */
#define CSR_DCSR_V_OFFSET                   5
#define CSR_DCSR_V_LENGTH                   1
#define CSR_DCSR_V                          (0x1U << CSR_DCSR_V_OFFSET)
/*
 * 0: \FcsrMstatusMprv in \Rmstatus is ignored in Debug Mode.
 *
 * 1: \FcsrMstatusMprv in \Rmstatus takes effect in Debug Mode.
 *
 * Implementing this bit is optional. It may be tied to either 0 or 1.
 */
#define CSR_DCSR_MPRVEN_OFFSET              4
#define CSR_DCSR_MPRVEN_LENGTH              1
#define CSR_DCSR_MPRVEN                     (0x1U << CSR_DCSR_MPRVEN_OFFSET)
/*
 * When set, there is a Non-Maskable-Interrupt (NMI) pending for the hart.
 *
 * Since an NMI can indicate a hardware error condition,
 * reliable debugging may no longer be possible once this bit becomes set.
 * This is implementation-dependent.
 */
#define CSR_DCSR_NMIP_OFFSET                3
#define CSR_DCSR_NMIP_LENGTH                1
#define CSR_DCSR_NMIP                       (0x1U << CSR_DCSR_NMIP_OFFSET)
/*
 * When set and not in Debug Mode, the hart will only execute a single
 * instruction and then enter Debug Mode. See Section~\ref{stepBit}
 * for details.
 *
 * The debugger must not change the value of this bit while the hart
 * is running.
 */
#define CSR_DCSR_STEP_OFFSET                2
#define CSR_DCSR_STEP_LENGTH                1
#define CSR_DCSR_STEP                       (0x1U << CSR_DCSR_STEP_OFFSET)
/*
 * Contains the privilege mode the hart was operating in when Debug
 * Mode was entered. The encoding is described in Table
 * \ref{tab:privmode}.  A debugger can change this value to change
 * the hart's privilege mode when exiting Debug Mode.
 *
 * Not all privilege modes are supported on all harts. If the
 * encoding written is not supported or the debugger is not allowed to
 * change to it, the hart may change to any supported privilege mode.
 */
#define CSR_DCSR_PRV_OFFSET                 0
#define CSR_DCSR_PRV_LENGTH                 2
#define CSR_DCSR_PRV                        (0x3U << CSR_DCSR_PRV_OFFSET)


#define DCSR_CAUSE_NONE     0
#define DCSR_CAUSE_SWBP     1
#define DCSR_CAUSE_HWBP     2
#define DCSR_CAUSE_DEBUGINT 3
#define DCSR_CAUSE_STEP     4
#define DCSR_CAUSE_HALT     5
#define DCSR_CAUSE_GROUP    6

#define PRV_U 0
#define PRV_S 1
#define PRV_M 3

// These are implementation-specific addresses in the Debug Module
#define DEBUG_ROM_HALTED    0x100
#define DEBUG_ROM_GOING     0x104
#define DEBUG_ROM_RESUMING  0x108
#define DEBUG_ROM_EXCEPTION 0x10C

// Region of memory where each hart has 1
// byte to read.
#define DEBUG_ROM_FLAGS 0x400
#define DEBUG_ROM_FLAG_GO     0
#define DEBUG_ROM_FLAG_RESUME 1

// These needs to match the link.ld
#define DEBUG_ROM_WHERETO 0x300
#define DEBUG_ROM_ENTRY   0x800
#define DEBUG_ROM_TVEC    0x808

#endif //MLDR187_SIMULATOR_BITS_H
