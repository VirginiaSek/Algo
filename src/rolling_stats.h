// compute statistics on a rolling basis.
// Based on the Welford algorithm, see https : // www.johndcook.com/blog/standard_deviation/

#ifndef ROLLING_STATS
#define ROLLING_STATS

typedef struct Stats
{
    long m_n;
    double m_oldM;
    double m_newM;
    double m_oldS;
    double m_newS;
} Stats;

void rolling_stats_reset(Stats *stats)
{
    stats->m_n = 0;
    stats->m_oldM = 0;
    stats->m_newM = 0;
    stats->m_oldS = 0;
    stats->m_newS = 0;
}

void rolling_stats_addValue(double x, Stats *stats)
{
    stats->m_n++;

    if (stats->m_n == 1)
    {
        stats->m_oldM = x;
        stats->m_newM = x;
        stats->m_oldS = 0.0;
    }
    else
    {
        stats->m_newM = stats->m_oldM + (x - stats->m_oldM) / (double)stats->m_n;
        stats->m_newS = stats->m_oldS + (x - stats->m_oldM) * (x - stats->m_newM);

        stats->m_oldM = stats->m_newM;
        stats->m_oldS = stats->m_newS;
    }
}

long rolling_stats_get_count(Stats *stats)
{
    return stats->m_n;
}

double rolling_stats_get_mean(Stats *stats)
{
    return stats->m_newM;
}

double rolling_stats_get_variance(Stats *stats, char sample)
{
    double n;
    if (sample)
    {
        n = stats->m_n - 1;
    }
    else
    {
        n = stats->m_n;
    }
    return stats->m_newS / (double)n;
}

#endif