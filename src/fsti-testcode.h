    struct fsti_simset simset;
    FILE *agents_in_file;
    char *agents_in_filename = "fsti_test_agents_in_1234.csv";
    FILE *config_file;
    char *config_filename = "fsti_test_config_1234.csv";
    const size_t num_agents = 101;

    const char *config_text;

    if (valgrind) {
        config_text =
            "[simulation_0]\n"
            "num_simulations=1\n"
            "num_agents=100\n"
            "before_events=_read_agents;_write_agents_csv_header;"
            "_write_partnerships_csv_header;_write_results_csv_header\n"
            "during_events=_age\n"
            "after_events=_report;_write_agents_csv\n"
            "agents_input_file=fsti_test_agents_in_1234.csv\n"
            "agents_output_file=fsti_test_agents_out_1234.csv\n"
            "partnerships_file=fsti_test_partnerships_1234.csv\n"
            "dataset_gen_mating=dataset_gen_mating.csv\n"
            "dataset_gen_infect=dataset_gen_infect.csv\n"
            "dataset_gen_treated=dataset_gen_treated.csv\n"
            "dataset_gen_resistant=dataset_gen_resistant.csv\n"
            "dataset_rel_period=dataset_rel.csv\n"
            "dataset_single_period=dataset_single.csv\n"
            "dataset_infect=dataset_infect.csv\n"
            "dataset_infect_stage=dataset_infect_stage.csv\n"
            "dataset_mortality=dataset_mortality_simple.csv\n"
            "results_file=fsti_test_results_1234.csv\n"
            "threads=1\n"
            "before_events=_write_agents_csv_header;_generate_agents;_"
            "initial_mating;_rkpm;_initial_rel;_report;_write_agents_csv\n"
            "during_events=_age;_breakup;_mating_pool;_shuffle_mating;_rkpm;"
            "_infect;_stage;_death;_birth\n"
            "record_matches=1\n"
            "record_breakups=1\n"
            "record_infections=1\n";
    } else {
        config_text =
            "[simulation_0]\n"
            "simulation_period=3652\n"
            "num_simulations=1\n"
            "before_events=_read_agents;_write_agents_csv_header;"
            "_write_partnerships_csv_header;_write_results_csv_header\n"
            "during_events=_age\n"
            "after_events=_report;_write_agents_csv\n"
            "agents_input_file=fsti_test_agents_in_1234.csv\n"
            "agents_output_file=fsti_test_agents_out_1234.csv\n"
            "partnerships_file=fsti_test_partnerships_1234.csv\n"

            "dataset_gen_sex=dataset_gen_sex.csv\n"
            "dataset_gen_sex_preferred=dataset_gen_sex_preferred.csv\n"
            "dataset_gen_infect=dataset_gen_infect.csv\n"
            "dataset_gen_treated=dataset_gen_treated.csv\n"
            "dataset_gen_resistant=dataset_gen_resistant.csv\n"
            "dataset_gen_mating=dataset_gen_mating.csv\n"

            "dataset_birth_infect=dataset_gen_infect.csv\n"
            "dataset_birth_treated=dataset_birth_treated.csv\n"
            "dataset_birth_resistant=dataset_birth_resistant.csv\n"

            "dataset_rel_period=dataset_rel.csv\n"
            "dataset_single_period=dataset_single.csv\n"
            "dataset_infect=dataset_infect.csv\n"
            "dataset_infect_stage=dataset_infect_stage.csv\n"
            "dataset_mortality=dataset_mortality_simple.csv\n"
            "results_file=fsti_test_results_1234.csv\n"
            "threads=1\n"
            "[simulation_1]\n"
            "threads=3\n"
            "num_simulations=5\n"
            "num_agents=500\n"
            "before_events=_write_agents_csv_header;_generate_agents;_"
            "initial_mating;_rkpm;_initial_rel;_report;_write_agents_csv\n"
            "during_events=_age;_breakup;_mating_pool;_shuffle_mating;_rkpm;"
            "_infect;_stage;_death;_birth\n"
            "# before_events=_generate_agents;_initial_mating;"
            "_rkpm;_initial_rel\n"
            "# during_events=_age;_breakup;_mating_pool;_shuffle_mating;_rkpm;"
            "_infect;_stage;_birth;_death\n"
            "match_k=1\n"
            "[simulation_2]\n"
            "num_simulations=1\n"
            "record_matches=1\n"
            "record_breakups=1\n"
            "record_infections=1\n"
            "match_k=100\n"
            "[simulation_3]\n"
            "time_step=DAY\n"
            "simulation_period=10-YEAR\n"
            "during_events=_age;_test_breakup;_test_mating_pool;"
           "_test_shuffle_mating;_test_rkpm;_test_infect;_stage;_test_birth;_test_death\n"
            "record_matches=0\n"
            "record_breakups=0\n"
            "record_infections=0\n"
            "match_k=10\n";
    }
    // Write an agents file
    agents_in_file = fopen(agents_in_filename, "w");
    assert(agents_in_file);
    fprintf(agents_in_file, "id;age;infected;sex;sex_preferred;partners_0\n");
    for (size_t i = 0; i < num_agents; i++) {
        double age = (double) rand() / RAND_MAX * 30.0 + 20.0;
        int infected = ( ( (double) rand()/RAND_MAX)  < 0.3) ? 1 : 0;
        int sex = ( ( (double) rand()/RAND_MAX) <0.5)
            ? FSTI_MALE : FSTI_FEMALE;
        int sex_preferred = ( ( (double) rand()/RAND_MAX)  < 0.06)
            ? sex : !sex;
        long partner;
        if (i % 2 == 0) {
            partner = -1;
        } else {
            if ( ( (double) rand() / RAND_MAX) < 0.5)
                partner = i - 1;
            else
                partner = -1;
        }
        fprintf(agents_in_file, "%zu;%.2f;%d;%d;%d;%ld\n",
                i,
                age,
                infected,
                sex,
                sex_preferred,
                partner);
    }
    fclose(agents_in_file);

    // Write config file
    config_file = fopen(config_filename, "w");
    assert(config_file);
    fprintf(config_file, "%s", config_text);
    fclose(config_file);

    fsti_events_tg = tg;

    fsti_simset_init(&simset);
    fsti_simset_load_config_file(&simset, config_filename);
    fsti_simset_exec(&simset);
    if (valgrind == false)
        TESTEQ(simset.sim_number, 8, *tg);
    fsti_simset_free(&simset);

    fsti_remove_file(agents_in_filename);
    fsti_remove_file(config_filename);
    fsti_remove_file("fsti_test_agents_out_1234.csv");
    fsti_remove_file("fsti_test_results_1234.csv");
    fsti_remove_file("fsti_test_partnerships_1234.csv");
