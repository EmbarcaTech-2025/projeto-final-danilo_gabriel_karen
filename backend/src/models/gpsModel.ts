import { DataTypes, Model } from 'sequelize';
import { sequelize } from '../config/db';

export class Session extends Model {
  declare id: number;
  declare category: string;
  declare notes?: string;
  declare startTime: Date;
  declare endTime?: Date;
}

Session.init(
  {
    id: {
      type: DataTypes.INTEGER,
      autoIncrement: true,
      primaryKey: true,
    },
    category: {
      type: DataTypes.STRING,
      allowNull: false,
    },
    notes: {
      type: DataTypes.TEXT,
    },
    startTime: {
      type: DataTypes.DATE,
      allowNull: false,
    },
    endTime: {
      type: DataTypes.DATE,
    }
  },
  {
    sequelize,
    modelName: 'Session',
    tableName: 'sessions',
    timestamps: false
  }
);