import api from "../api/axios";
import endpoints from "../api/endpoints";

const weeklyPassagesService = {
  getAll: async () => {
    const response = await api.get(endpoints.dashboard.weeklyPassages);
    return response.data;
  },
};

export default weeklyPassagesService;
