#include <Arduino.h>
#include <unity.h>
#include <LittleFS.h>
#include "../include/log_format.h"

// setUp/tearDown : exécutés avant/après CHAQUE test, pour garantir l'isolation.
void setUp(void)
{
    TEST_ASSERT_TRUE_MESSAGE(storage_init(), "storage_init a echoue dans setUp()");

    if (LittleFS.exists("/logs.csv"))
    {
        LittleFS.remove("/logs.csv");
        File f = LittleFS.open("/logs.csv", "w");
        if (f)
            f.close();
    }
}

void tearDown(void)
{
    // Rien a nettoyer : chaque setUp() repart d'un fichier vide.
}

// storage_init
void test_storage_init_creates_empty_file(void)
{
    TEST_ASSERT_TRUE(LittleFS.exists("/logs.csv"));
    File f = LittleFS.open("/logs.csv", "r");
    TEST_ASSERT_EQUAL_UINT32(0, f.size());
    f.close();
}

// storage_append_log
void test_append_log_assigns_sequential_line_index(void)
{
    uint32_t idx0, idx1, idx2;
    TEST_ASSERT_TRUE(storage_append_log("2026-08-30T11:42:07Z", "PN-01", "00112233445566", &idx0));
    TEST_ASSERT_TRUE(storage_append_log("2026-08-30T11:45:12Z", "PN-01", "AABBCCDD", &idx1));
    TEST_ASSERT_TRUE(storage_append_log("2026-08-30T11:50:00Z", "PN-01", "0102030405", &idx2));

    TEST_ASSERT_EQUAL_UINT32(0, idx0);
    TEST_ASSERT_EQUAL_UINT32(1, idx1);
    TEST_ASSERT_EQUAL_UINT32(2, idx2);
}

void test_append_log_writes_exact_line_length(void)
{
    storage_append_log("2026-08-30T11:42:07Z", "PN-01", "00112233445566", nullptr);

    File f = LittleFS.open("/logs.csv", "r");
    TEST_ASSERT_EQUAL_UINT32(LOG_LINE_LEN, f.size());
    f.close();
}

void test_append_log_rejects_invalid_timestamp_length(void)
{
    uint32_t idx;
    bool ok = storage_append_log("2026-08-30", "PN-01", "00112233445566", &idx); // trop court
    TEST_ASSERT_FALSE(ok);
}

void test_append_log_pads_agent_and_checkpoint_correctly(void)
{
    storage_append_log("2026-08-30T11:42:07Z", "A", "1", nullptr);

    LogEntry entries[1];
    size_t n = storage_find_by_status(LOG_STATUS_PENDING, entries, 1);
    TEST_ASSERT_EQUAL_UINT32(1, n);

    TEST_ASSERT_EQUAL_STRING("A    ", entries[0].id_agent);               // pad droite espaces
    TEST_ASSERT_EQUAL_STRING("00000000000001", entries[0].id_checkpoint); // pad gauche zeros
}

// storage_update_status
void test_update_status_changes_only_target_line(void)
{
    uint32_t idx0, idx1;
    storage_append_log("2026-08-30T11:42:07Z", "PN-01", "00112233445566", &idx0);
    storage_append_log("2026-08-30T11:45:12Z", "PN-01", "AABBCCDD", &idx1);

    TEST_ASSERT_TRUE(storage_update_status(idx1, LOG_STATUS_SENT));

    LogEntry pending[10];
    TEST_ASSERT_EQUAL_UINT32(1, storage_find_by_status(LOG_STATUS_PENDING, pending, 10));
    TEST_ASSERT_EQUAL_UINT32(idx0, pending[0].line_index);

    LogEntry sent[10];
    TEST_ASSERT_EQUAL_UINT32(1, storage_find_by_status(LOG_STATUS_SENT, sent, 10));
    TEST_ASSERT_EQUAL_UINT32(idx1, sent[0].line_index);
}

void test_update_status_fails_for_out_of_range_index(void)
{
    storage_append_log("2026-08-30T11:42:07Z", "PN-01", "00112233445566", nullptr);
    TEST_ASSERT_FALSE(storage_update_status(999, LOG_STATUS_SENT));
}

void test_update_status_does_not_change_file_size(void)
{
    uint32_t idx0;
    storage_append_log("2026-08-30T11:42:07Z", "PN-01", "00112233445566", &idx0);

    File f1 = LittleFS.open("/logs.csv", "r");
    uint32_t sizeBefore = f1.size();
    f1.close();

    storage_update_status(idx0, LOG_STATUS_SENT);

    File f2 = LittleFS.open("/logs.csv", "r");
    uint32_t sizeAfter = f2.size();
    f2.close();

    TEST_ASSERT_EQUAL_UINT32(sizeBefore, sizeAfter);
}

// storage_find_by_status
void test_find_by_status_returns_only_matching_entries(void)
{
    uint32_t idx0, idx1, idx2;
    storage_append_log("2026-08-30T11:00:00Z", "PN-01", "0000000000001", &idx0);
    storage_append_log("2026-08-30T11:01:00Z", "PN-01", "0000000000002", &idx1);
    storage_append_log("2026-08-30T11:02:00Z", "PN-01", "0000000000003", &idx2);
    storage_update_status(idx1, LOG_STATUS_FAILED);

    LogEntry pending[10];
    TEST_ASSERT_EQUAL_UINT32(2, storage_find_by_status(LOG_STATUS_PENDING, pending, 10));

    LogEntry failed[10];
    TEST_ASSERT_EQUAL_UINT32(1, storage_find_by_status(LOG_STATUS_FAILED, failed, 10));
    TEST_ASSERT_EQUAL_UINT32(idx1, failed[0].line_index);
}

void test_find_by_status_respects_max_entries_limit(void)
{
    for (int i = 0; i < 5; i++)
    {
        char ts[21];
        snprintf(ts, sizeof(ts), "2026-08-30T11:%02d:00Z", i);
        storage_append_log(ts, "PN-01", "0000000000001", nullptr);
    }

    LogEntry limited[2];
    size_t n = storage_find_by_status(LOG_STATUS_PENDING, limited, 2);
    TEST_ASSERT_EQUAL_UINT32(2, n); // borne par max_entries, pas par le contenu reel (5)
}

// storage_build_log_id
void test_build_log_id_strips_separators_from_timestamp(void)
{
    LogEntry entry{};
    strcpy(entry.timestamp_iso, "2026-08-30T11:42:07Z");
    strcpy(entry.id_checkpoint, "00112233445566");

    char logId[64];
    storage_build_log_id(entry, logId, sizeof(logId));

    TEST_ASSERT_EQUAL_STRING("20260830T114207Z-00112233445566", logId);
}

void test_build_log_id_is_stable_for_same_entry(void)
{
    LogEntry entry{};
    strcpy(entry.timestamp_iso, "2026-08-30T11:42:07Z");
    strcpy(entry.id_checkpoint, "00112233445566");

    char idA[64], idB[64];
    storage_build_log_id(entry, idA, sizeof(idA));
    storage_build_log_id(entry, idB, sizeof(idB));

    TEST_ASSERT_EQUAL_STRING(idA, idB);
}

// storage_is_space_low
void test_is_space_low_true_with_zero_threshold(void)
{
    TEST_ASSERT_TRUE(storage_is_space_low(0));
}

void test_is_space_low_false_with_max_threshold_on_fresh_fs(void)
{
    TEST_ASSERT_FALSE(storage_is_space_low(100));
}

// storage_reclaim_space
void test_reclaim_space_removes_only_sent_entries(void)
{
    uint32_t idx0, idx1, idx2;
    storage_append_log("2026-08-30T11:00:00Z", "PN-01", "0000000000001", &idx0);
    storage_append_log("2026-08-30T11:01:00Z", "PN-01", "0000000000002", &idx1);
    storage_append_log("2026-08-30T11:02:00Z", "PN-01", "0000000000003", &idx2);
    storage_update_status(idx1, LOG_STATUS_SENT);

    TEST_ASSERT_TRUE(storage_reclaim_space());

    File f = LittleFS.open("/logs.csv", "r");
    TEST_ASSERT_EQUAL_UINT32(2 * LOG_LINE_LEN, f.size());
    f.close();

    LogEntry sent[10];
    TEST_ASSERT_EQUAL_UINT32(0, storage_find_by_status(LOG_STATUS_SENT, sent, 10));

    LogEntry pending[10];
    TEST_ASSERT_EQUAL_UINT32(2, storage_find_by_status(LOG_STATUS_PENDING, pending, 10));
}

void test_reclaim_space_shifts_remaining_line_indexes(void)
{
    uint32_t idx0, idx1, idx2;
    storage_append_log("2026-08-30T11:00:00Z", "PN-01", "0000000000001", &idx0); // sera supprimee
    storage_append_log("2026-08-30T11:01:00Z", "PN-01", "0000000000002", &idx1);
    storage_append_log("2026-08-30T11:02:00Z", "PN-01", "0000000000003", &idx2);
    storage_update_status(idx0, LOG_STATUS_SENT);

    storage_reclaim_space();

    LogEntry pending[10];
    size_t n = storage_find_by_status(LOG_STATUS_PENDING, pending, 10);
    TEST_ASSERT_EQUAL_UINT32(2, n);
    // Apres suppression de la ligne 0, les entrees restantes sont decalees.
    TEST_ASSERT_EQUAL_UINT32(0, pending[0].line_index);
    TEST_ASSERT_EQUAL_UINT32(1, pending[1].line_index);
}

void test_reclaim_space_survives_large_volume_without_reset(void)
{
    for (int i = 0; i < 500; i++)
    {
        char ts[21];
        snprintf(ts, sizeof(ts), "2026-08-30T%02d:%02d:%02dZ", (i / 3600) % 24, (i / 60) % 60, i % 60);
        storage_append_log(ts, "PN-01", "0000000000001", nullptr);
    }
    TEST_ASSERT_TRUE(storage_reclaim_space());
}

// Runner Unity
void setup()
{
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(test_storage_init_creates_empty_file);

    RUN_TEST(test_append_log_assigns_sequential_line_index);
    RUN_TEST(test_append_log_writes_exact_line_length);
    RUN_TEST(test_append_log_rejects_invalid_timestamp_length);
    RUN_TEST(test_append_log_pads_agent_and_checkpoint_correctly);

    RUN_TEST(test_update_status_changes_only_target_line);
    RUN_TEST(test_update_status_fails_for_out_of_range_index);
    RUN_TEST(test_update_status_does_not_change_file_size);

    RUN_TEST(test_find_by_status_returns_only_matching_entries);
    RUN_TEST(test_find_by_status_respects_max_entries_limit);

    RUN_TEST(test_build_log_id_strips_separators_from_timestamp);
    RUN_TEST(test_build_log_id_is_stable_for_same_entry);

    RUN_TEST(test_is_space_low_true_with_zero_threshold);
    RUN_TEST(test_is_space_low_false_with_max_threshold_on_fresh_fs);

    RUN_TEST(test_reclaim_space_removes_only_sent_entries);
    RUN_TEST(test_reclaim_space_shifts_remaining_line_indexes);
    RUN_TEST(test_reclaim_space_survives_large_volume_without_reset);

    UNITY_END();
}

void loop()
{
    // Rien : tous les tests s'executent une seule fois dans setup().
}