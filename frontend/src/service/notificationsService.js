import api from "./api";
import endpoints from "./endpoints";

const notificationsService = {
  getAll: async (params = {}) => {
    const response = await api.get(endpoints.notifications.list, { params });
    return response.data;
  },

  getById: async (id) => {
    const response = await api.get(endpoints.notifications.detail(id));
    return response.data;
  },

  archive: async (id) => {
    const response = await api.patch(endpoints.notifications.archive(id));
    return response.data;
  },
};

export default notificationsService;
