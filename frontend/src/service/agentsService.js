import api from "../api/axios";
import endpoints from "../api/endpoints";

const agentsService = {
  getAll: async (params = {}) => {
    const response = await api.get(endpoints.agents.list, { params });
    return response.data;
  },

  getById: async (id) => {
    const response = await api.get(endpoints.agents.detail(id));
    return response.data;
  },

  create: async (agentData) => {
    const response = await api.post(endpoints.agents.create, agentData);
    return response.data;
  },

  update: async (id, agentData) => {
    const response = await api.patch(endpoints.agents.update(id), agentData);
    return response.data;
  },

  updateStatus: async (id, statusData) => {
    const response = await api.patch(
      endpoints.agents.updateStatus(id),
      statusData,
    );
    return response.data;
  },

  delete: async (id) => {
    const response = await api.delete(endpoints.agents.delete(id));
    return response.data;
  },
};

export default agentsService;
