#include <unistd.h>
#include <glib.h>
#include "fsti-simset.h"
#include "fsti-error.h"
#include "fsti-agent.h"
#include "fsti-defaults.h"
#include "fsti-procini.h"
#include "fsti-events.h"
#include "fsti-config.h"

static GMutex thread_mutex;
static GCond thread_cond;
static unsigned thread_no = 0;

static void init(struct fsti_simset *simset)
{
    fsti_config_init(&simset->config);
    fsti_agent_arr_init(&fsti_saved_agent_arr);
    simset->config_strings = NULL;
    simset->more_configs = true;
    simset->config_num_sims = 0;
    simset->config_sim_number = 0;
    simset->sim_number = 0;
    simset->groups = NULL;
    simset->group_ptr = NULL;
    simset->csv = NULL;
    simset->close_results_file = simset->close_agents_output_file = false;
    fsti_event_register_standard_events();
    simset->key_file = g_key_file_new();
    FSTI_ASSERT(simset->key_file, FSTI_ERR_KEY_FILE_OPEN, "\n");
}

void fsti_simset_init_with_config(struct fsti_simset *simset,
                                  const struct fsti_config *config)
{
    init(simset);
    simset->more_configs = true;
    fsti_config_copy(&simset->config, config);
}

void fsti_simset_init(struct fsti_simset *simset)
{
    init(simset);
    fsti_config_set_default(&simset->config);
}

void fsti_simset_free(struct fsti_simset *simset)
{
    fsti_config_free(&simset->config);
    g_strfreev(simset->groups);
    g_key_file_free(simset->key_file);
    fsti_agent_arr_free(&fsti_saved_agent_arr);
    fsti_register_free();
}

void fsti_simset_load_config_file(struct fsti_simset *simset,
                                  const char *filename)
{
    fsti_read_ini(filename, simset->key_file);

    simset->groups = g_key_file_get_groups(simset->key_file, NULL);
    simset->group_ptr = simset->groups;
    simset->more_configs = true;
}

void fsti_simset_load_config_strings(struct fsti_simset *simset,
				     char **config_strings)
{
    simset->config_strings = config_strings;
}

void fsti_simset_set_csv(struct fsti_simset *simset,
                         const struct fsti_csv_agent *csv)
{
    simset->csv = csv;
}

static void set_keys(struct fsti_simset *simset)
{
    GError *error = NULL;
    char **keys = g_key_file_get_keys(simset->key_file, *simset->group_ptr,
                                      NULL, &error);
    FSTI_ASSERT(error == NULL, FSTI_ERR_KEY_FILE_SYNTAX,
                error->message);
    FSTI_ASSERT(keys && keys[0], FSTI_ERR_KEY_FILE_GROUP_EMPTY,
                FSTI_MSG("At group ", *simset->group_ptr));
    for (char **key = keys; *key != NULL; ++key) {
        char *values = g_key_file_get_value(simset->key_file,
                                            *simset->group_ptr,
                                            *key, &error);
        FSTI_ASSERT(error == NULL, FSTI_ERR_KEY_FILE_SYNTAX,
                    *simset->group_ptr);
        fsti_config_replace_values(&simset->config,
                                   *key, values);
        free(values);
    }
    g_strfreev(keys);
    simset->config_num_sims = (size_t)
        fsti_config_at0_long(&simset->config,"NUM_SIMULATIONS");
    FSTI_ASSERT(simset->config_num_sims > 0, FSTI_ERR_INVALID_VALUE, NULL);
}

static void set_output_files(struct fsti_simset *simset,
                             struct fsti_simulation *simulation)
{
    char *results_file_name,  *agents_file_name;

    if (simset->sim_number == 0) {
        results_file_name = fsti_config_at0_str(&simset->config,
                                                "RESULTS_FILE");

        if(strcmp(results_file_name, "")) {
            simset->results_file =
                simulation->results_file = fopen(results_file_name, "w");
            FSTI_ASSERT(simulation->results_file, FSTI_ERR_FILE, strerror(errno));
            simset->close_results_file = true;
        }

        agents_file_name = fsti_config_at0_str(&simset->config,
                                               "AGENTS_OUTPUT_FILE");
        if(strcmp(agents_file_name, "")) {
            simset->agents_output_file =
                simulation->agents_output_file = fopen(agents_file_name, "w");
            FSTI_ASSERT(simulation->agents_output_file, FSTI_ERR_FILE,
                        strerror(errno));
            simset->close_agents_output_file = true;
        }
    }
}

static void update_config(struct fsti_simset *simset)
{

    if (simset->config_sim_number == 0) {
        set_keys(simset);
    } else if (simset->config_sim_number >= simset->config_num_sims) {
        ++simset->group_ptr;
        simset->config_sim_number = 0;
        if (*simset->group_ptr) {
            update_config(simset);
        }
    }
}

static void *threaded_sim(void *simulation)
{
    fsti_simulation_run(simulation);
    fsti_simulation_free(simulation);
    free(simulation);
    g_mutex_lock(&thread_mutex);
    thread_no--;
    g_cond_signal (&thread_cond);
    g_mutex_unlock(&thread_mutex);
    return NULL;
}

void fsti_simset_exec(struct fsti_simset *simset)
{
    char thread_name[10];
    struct fsti_simulation *simulation;
    unsigned max_threads;
    GThread *thread;

    if (simset->group_ptr == NULL || *simset->group_ptr == NULL)
        return;

    set_keys(simset);
    max_threads = (unsigned)
        fsti_config_at0_long(&simset->config,"THREADS");
    if (max_threads == 0)
        max_threads = g_get_num_processors();

    while (*simset->group_ptr) {
        simulation = malloc(sizeof(*simulation));
        FSTI_ASSERT(simulation, FSTI_ERR_NOMEM, NULL);
        fsti_simulation_init(simulation, &simset->config,
                             simset->sim_number,
                             simset->config_sim_number);
        fsti_simulation_set_csv(simulation, simset->csv);
        set_output_files(simset, simulation);
        simulation->name = *simset->group_ptr;
        if (max_threads > 1) {
            snprintf(thread_name, 10, "%u", thread_no);
            g_mutex_lock(&thread_mutex);
            while (thread_no >= max_threads)
                g_cond_wait (&thread_cond, &thread_mutex);
            thread = g_thread_new(thread_name, threaded_sim, simulation);
            g_thread_unref(thread);
            thread_no++;
            g_mutex_unlock(&thread_mutex);
        } else {
            threaded_sim(simulation);
        }
        ++simset->config_sim_number;
        ++simset->sim_number;
        update_config(simset);
    }
    if (max_threads > 1) {
        g_mutex_lock(&thread_mutex);
        while (thread_no > 0)
            g_cond_wait (&thread_cond, &thread_mutex);
        g_mutex_unlock(&thread_mutex);
        if (simset->close_results_file) fclose(simset->results_file);
        if (simset->close_agents_output_file) fclose(simset->agents_output_file);
    }
}
