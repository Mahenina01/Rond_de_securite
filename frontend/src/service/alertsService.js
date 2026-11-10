import api from "../api/axios";
import endpoints from "../api/endpoints";

const alertsService = {
  getAll: async (params = {}) => {
    const response = await api.get(endpoints.alerts.list, { params });
    return response.data;
  },

  getById: async (id) => {
    const response = await api.get(endpoints.alerts.detail(id));
    return response.data;
  },

  acknowledge: async (id) => {
    const response = await api.patch(endpoints.alerts.acknowledge(id));
    return response.data;
  },

  resolve: async (id) => {
    const response = await api.patch(endpoints.alerts.resolve(id));
    return response.data;
  },

  delete: async (id) => {
    await api.delete(endpoints.alerts.delete(id));
  },
};

export default alertsService;
