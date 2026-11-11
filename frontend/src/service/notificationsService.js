import api from "../api/axios";
import endpoints from "../api/endpoints";

const notificationsService = {
  getActive: async () => {
    const response = await api.get(endpoints.notifications.list, {
      params: { status: "ACTIVE" },
    });
    return response.data;
  },

  getAll: async (params = {}) => {
    const response = await api.get(endpoints.notifications.list, { params });
    return response.data;
  },

  getById: async (id) => {
    const response = await api.get(endpoints.notifications.detail(id));
    return response.data;
  },

  updateReadStatus: async (id, read) => {
    const response = await api.patch(endpoints.notifications.read(id), { read });
    return response.data;
  },

  archive: async (id) => {
    const response = await api.patch(endpoints.notifications.archive(id));
    return response.data;
  },
};

export default notificationsService;
