import api from "../api/axios";
import endpoints from "../api/endpoints";

const logsService = {
  getAll: async (params = {}) => {
    const response = await api.get(endpoints.logs.list, { params });
    return response.data;
  },

  getById: async (id) => {
    const response = await api.get(endpoints.logs.detail(id));
    return response.data;
  },

  delete: async (id) => {
    const response = await api.delete(endpoints.logs.delete(id));
    return response.data;
  },
};

export default logsService;
