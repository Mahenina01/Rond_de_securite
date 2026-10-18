import api from "../api/axios";
import endpoints from "../api/endpoints";

const locationsService = {
  getAll: async (params = {}) => {
    const response = await api.get(endpoints.locations.list, { params });
    return response.data;
  },

  search: async (query) => {
    const response = await api.get(endpoints.locations.search, {
      params: { search: query },
    });
    return response.data;
  },

  getById: async (id) => {
    const response = await api.get(endpoints.locations.detail(id));
    return response.data;
  },

  getByAgentId: async (agentId) => {
    const response = await api.get(endpoints.locations.agentId(agentId));
    return response.data;
  },

  create: async (locationData) => {
    const response = await api.post(endpoints.locations.create, locationData);
    return response.data;
  },

  update: async (id, locationData) => {
    const response = await api.put(
      endpoints.locations.update(id),
      locationData,
    );
    return response.data;
  },

  delete: async (id) => {
    const response = await api.delete(endpoints.locations.delete(id));
    return response.data;
  },
};

export default locationsService;
