import api from "../api/axios";
import endpoints from "../api/endpoints";

const devicesService = {
  getAll: async (params = {}) => {
    const response = await api.get(endpoints.devices.list, { params });
    return response.data;
  },

  getById: async (id) => {
    const response = await api.get(endpoints.devices.detail(id));
    return response.data;
  },

  create: async (deviceData) => {
    const response = await api.post(endpoints.devices.create, deviceData);
    return response.data;
  },

  update: async (id, deviceData) => {
    const response = await api.patch(endpoints.devices.update(id), deviceData);
    return response.data;
  },

  delete: async (id) => {
    const response = await api.delete(endpoints.devices.delete(id));
    return response.data;
  },
};

export default devicesService;
