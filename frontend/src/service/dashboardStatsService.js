import api from "../api/axios";
import endpoints from "../api/endpoints";

const dashboardStatsService = {
  get: async () => {
    const response = await api.get(endpoints.dashboard.stats);
    return response.data;
  },
};

export default dashboardStatsService;
