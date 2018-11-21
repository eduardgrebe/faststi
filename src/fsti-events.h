#ifndef FSTI_EVENTS_H
#define FSTI_EVENTS_H

#include <stdint.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include "fsti-simulation.h"
#include "fsti-defs.h"
#include "fsti-report.h"
#include "fsti-agent.h"
#include "fsti-userdefs.h"

void fsti_event_read_agents(struct fsti_simulation *simulation);
void fsti_event_create_agents(struct fsti_simulation *simulation);
void fsti_event_age(struct fsti_simulation *simulation);
void fsti_event_report(struct fsti_simulation *simulation);
void fsti_event_flex_report(struct fsti_simulation *simulation);
void fsti_event_shuffle_living(struct fsti_simulation *simulation);
void fsti_event_shuffle_mating(struct fsti_simulation *simulation);
void fsti_event_write_results_csv_header(struct fsti_simulation *simulation);
void fsti_event_write_agents_csv_header(struct fsti_simulation *simulation);
void fsti_event_write_agents_csv(struct fsti_simulation *simulation);
void fsti_event_write_living_agents_csv(struct fsti_simulation *simulation);
void fsti_event_write_dead_agents_csv(struct fsti_simulation *simulation);
void fsti_event_write_agents_pretty(struct fsti_simulation *simulation);
void fsti_event_no_op(struct fsti_simulation *simulation);
void fsti_event_register_events();

extern struct fsti_agent fsti_global_agent;
extern const struct fsti_csv_entry fsti_csv_entries[];

extern const struct fsti_csv_agent fsti_global_csv;

#endif
